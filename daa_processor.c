#include "daa_processor.h"
#include <stdio.h>
#include <string.h>

/******************************************************************************
 * Initialize the Circular Buffer
 ******************************************************************************/
void InitDAAProcessor(DAACircularBuffer *cbuf) {
    memset(cbuf, 0, sizeof(DAACircularBuffer));
    pthread_mutex_init(&cbuf->mutex, NULL);
    pthread_cond_init(&cbuf->cond, NULL);
}

/******************************************************************************
 * Merge ADS-B Data into Circular Buffer
 ******************************************************************************/
void MergeADSBData(const ADSBMessage *adsb_msg, DAACircularBuffer *cbuf) {
    if (adsb_msg == NULL || !adsb_msg->is_valid) {
        return;
    }

    pthread_mutex_lock(&cbuf->mutex);

    // Prepare a merged track
    DAAMergedTrack merged_track = {
        .source = 0, // Source: ADS-B
        .track_id = adsb_msg->icao_address,
        .latitude = adsb_msg->latitude,
        .longitude = adsb_msg->longitude,
        .altitude = adsb_msg->altitude * 0.3048f, // Convert feet to meters
        .velocity = adsb_msg->velocity * 0.514444f, // Convert knots to m/s
        .heading = adsb_msg->heading,
        .confidence = 100,
        .is_valid = 1
    };

    // Add the track to the circular buffer
    cbuf->buffer[cbuf->tail] = merged_track;
    cbuf->tail = (cbuf->tail + 1) % DAA_BUFFER_SIZE;
    if (cbuf->count == DAA_BUFFER_SIZE) {
        cbuf->head = (cbuf->head + 1) % DAA_BUFFER_SIZE; // Overwrite oldest
    } else {
        cbuf->count++;
    }

    pthread_cond_signal(&cbuf->cond);
    pthread_mutex_unlock(&cbuf->mutex);
}

/******************************************************************************
 * Merge Radar Data into Circular Buffer
 ******************************************************************************/
void MergeRadarData(const RadarTrack *radar_track, DAACircularBuffer *cbuf) {
    if (radar_track == NULL || !radar_track->is_valid) {
        return;
    }

    pthread_mutex_lock(&cbuf->mutex);

    // Prepare a merged track
    DAAMergedTrack merged_track = {
        .source = 1, // Source: Radar
        .track_id = radar_track->track_id,
        .latitude = radar_track->latitude,
        .longitude = radar_track->longitude,
        .altitude = radar_track->altitude,
        .velocity = radar_track->velocity,
        .heading = radar_track->heading,
        .confidence = radar_track->confidence,
        .is_valid = 1
    };

    // Add the track to the circular buffer
    cbuf->buffer[cbuf->tail] = merged_track;
    cbuf->tail = (cbuf->tail + 1) % DAA_BUFFER_SIZE;
    if (cbuf->count == DAA_BUFFER_SIZE) {
        cbuf->head = (cbuf->head + 1) % DAA_BUFFER_SIZE; // Overwrite oldest
    } else {
        cbuf->count++;
    }

    pthread_cond_signal(&cbuf->cond);
    pthread_mutex_unlock(&cbuf->mutex);
}

/******************************************************************************
 * Get Merged Data from the Circular Buffer
 ******************************************************************************/
int GetMergedData(DAACircularBuffer *cbuf, DAAMergedTrack *output) {
    pthread_mutex_lock(&cbuf->mutex);

    while (cbuf->count == 0) {
        pthread_cond_wait(&cbuf->cond, &cbuf->mutex); // Wait for data
    }

    // Retrieve the oldest track
    *output = cbuf->buffer[cbuf->head];
    cbuf->head = (cbuf->head + 1) % DAA_BUFFER_SIZE;
    cbuf->count--;

    pthread_mutex_unlock(&cbuf->mutex);
    return 1; // Successfully retrieved data
}
