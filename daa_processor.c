#include "daa_processor.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAX_DISTANCE_THRESHOLD 0.01 // Maximum allowable distance difference (in degrees) for matching tracks
#define MAX_ALTITUDE_DIFF 50.0      // Maximum allowable altitude difference (in meters) for matching tracks

/******************************************************************************
 * Initialize the Circular Buffer
 ******************************************************************************/
void InitDAAProcessor(DAACircularBuffer *cbuf) {
    memset(cbuf, 0, sizeof(DAACircularBuffer));
    pthread_mutex_init(&cbuf->mutex, NULL);
    pthread_cond_init(&cbuf->cond, NULL);
}

/******************************************************************************
 * Calculate Distance Between Two Points (Haversine Formula Approximation)
 ******************************************************************************/
float CalculateDistance(float lat1, float lon1, float lat2, float lon2) {
    return sqrtf(powf(lat1 - lat2, 2) + powf(lon1 - lon2, 2)); // Simple 2D distance approximation
}

/******************************************************************************
 * Fusion Algorithm: Merge Radar Data into Circular Buffer
 ******************************************************************************/
void MergeRadarData(const RadarTrack *radar_track, DAACircularBuffer *cbuf) {
    if (radar_track == NULL || !radar_track->is_valid) {
        return;
    }

    pthread_mutex_lock(&cbuf->mutex);

    int is_merged = 0;

    // Attempt to match radar data with an existing ADS-B track in the buffer
    for (int i = 0; i < cbuf->count; i++) {
        int idx = (cbuf->head + i) % DAA_BUFFER_SIZE;
        DAAMergedTrack *existing_track = &cbuf->buffer[idx];

        if (existing_track->source == 0) { // ADS-B source
            float distance = CalculateDistance(
                radar_track->latitude, radar_track->longitude,
                existing_track->latitude, existing_track->longitude
            );

            if (distance <= MAX_DISTANCE_THRESHOLD &&
                fabsf(radar_track->altitude - existing_track->altitude) <= MAX_ALTITUDE_DIFF) {
                // Fusion: Update precision of ADS-B track using radar data
                existing_track->altitude = radar_track->altitude;
                existing_track->confidence = (existing_track->confidence + radar_track->confidence) / 2;
                is_merged = 1;
                break;
            }
        }
    }

    // If not merged, add the radar track as a new entry
    if (!is_merged) {
        DAAMergedTrack new_track = {
            .source = 1, // Radar
            .track_id = radar_track->track_id,
            .latitude = radar_track->latitude,
            .longitude = radar_track->longitude,
            .altitude = radar_track->altitude,
            .velocity = radar_track->velocity,
            .heading = radar_track->heading,
            .confidence = radar_track->confidence,
            .is_valid = 1
        };

        cbuf->buffer[cbuf->tail] = new_track;
        cbuf->tail = (cbuf->tail + 1) % DAA_BUFFER_SIZE;
        if (cbuf->count == DAA_BUFFER_SIZE) {
            cbuf->head = (cbuf->head + 1) % DAA_BUFFER_SIZE; // Overwrite oldest
        } else {
            cbuf->count++;
        }
    }

    pthread_cond_signal(&cbuf->cond);
    pthread_mutex_unlock(&cbuf->mutex);
}

/******************************************************************************
 * Fusion Algorithm: Merge ADS-B Data into Circular Buffer
 ******************************************************************************/
void MergeADSBData(const ADSBMessage *adsb_msg, DAACircularBuffer *cbuf) {
    if (adsb_msg == NULL || !adsb_msg->is_valid) {
        return;
    }

    pthread_mutex_lock(&cbuf->mutex);

    int is_existing = 0;

    // Attempt to find an existing ADS-B track
    for (int i = 0; i < cbuf->count; i++) {
        int idx = (cbuf->head + i) % DAA_BUFFER_SIZE;
        DAAMergedTrack *existing_track = &cbuf->buffer[idx];

        if (existing_track->source == 0 && existing_track->track_id == adsb_msg->icao_address) {
            // Update existing ADS-B track
            existing_track->latitude = adsb_msg->latitude;
            existing_track->longitude = adsb_msg->longitude;
            existing_track->altitude = adsb_msg->altitude * 0.3048f; // Feet to meters
            existing_track->velocity = adsb_msg->velocity * 0.514444f; // Knots to m/s
            existing_track->heading = adsb_msg->heading;
            is_existing = 1;
            break;
        }
    }

    // If not found, add as a new track
    if (!is_existing) {
        DAAMergedTrack new_track = {
            .source = 0, // ADS-B
            .track_id = adsb_msg->icao_address,
            .latitude = adsb_msg->latitude,
            .longitude = adsb_msg->longitude,
            .altitude = adsb_msg->altitude * 0.3048f, // Feet to meters
            .velocity = adsb_msg->velocity * 0.514444f, // Knots to m/s
            .heading = adsb_msg->heading,
            .confidence = 100,
            .is_valid = 1
        };

        cbuf->buffer[cbuf->tail] = new_track;
        cbuf->tail = (cbuf->tail + 1) % DAA_BUFFER_SIZE;
        if (cbuf->count == DAA_BUFFER_SIZE) {
            cbuf->head = (cbuf->head + 1) % DAA_BUFFER_SIZE; // Overwrite oldest
        } else {
            cbuf->count++;
        }
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
