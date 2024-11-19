#include "ecoflight_radar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

/******************************************************************************
 * Initialize the Radar Driver
 ******************************************************************************/
void InitRadarDriver(RadarState *state) {
    if (state == NULL) {
        fprintf(stderr, "RadarState cannot be NULL.\n");
        exit(EXIT_FAILURE);
    }

    // Initialize socket
    state->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (state->sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    memset(&state->server_addr, 0, sizeof(state->server_addr));
    state->server_addr.sin_family = AF_INET;
    state->server_addr.sin_addr.s_addr = INADDR_ANY;
    state->server_addr.sin_port = htons(RADAR_PORT);

    // Bind the socket
    if (bind(state->sockfd, (const struct sockaddr *)&state->server_addr, sizeof(state->server_addr)) < 0) {
        perror("Bind failed");
        close(state->sockfd);
        exit(EXIT_FAILURE);
    }

    // Initialize state
    state->num_tracks = 0;
    pthread_mutex_init(&state->state_mutex, NULL);

    printf("EcoFlight Radar Driver initialized on port %d\n", RADAR_PORT);
}

/******************************************************************************
 * Process Raw Radar Data
 ******************************************************************************/
void ProcessRadarData(const char *raw_data, RadarState *state) {
    if (raw_data == NULL || state == NULL) {
        return;
    }

    pthread_mutex_lock(&state->state_mutex);

    // Clear current tracks
    state->num_tracks = 0;

    // Example parsing logic (mocked): Replace with actual radar decoding logic
    uint32_t num_tracks = 0;
    sscanf(raw_data, "%u", &num_tracks);
    state->num_tracks = num_tracks > MAX_TRACKS ? MAX_TRACKS : num_tracks;

    const char *track_data = raw_data + sizeof(uint32_t);
    for (uint32_t i = 0; i < state->num_tracks; i++) {
        RadarTrack *track = &state->tracks[i];
        sscanf(track_data, "%u,%f,%f,%f,%f,%f,%hhu",
               &track->track_id,
               &track->latitude,
               &track->longitude,
               &track->altitude,
               &track->velocity,
               &track->heading,
               &track->confidence);

        track->is_valid = (track->confidence > 50); // Example validity check
        track_data += sizeof(RadarTrack); // Adjust based on actual format
    }

    pthread_mutex_unlock(&state->state_mutex);
}

/******************************************************************************
 * Receive Radar Packets
 ******************************************************************************/
void ReceiveRadarPackets(RadarState *state) {
    char buffer[RADAR_BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    printf("Waiting for radar packets...\n");

    while (1) {
        ssize_t n = recvfrom(state->sockfd, buffer, RADAR_BUFFER_SIZE, 0,
                             (struct sockaddr *)&client_addr, &addr_len);
        if (n < 0) {
            perror("recvfrom failed");
            continue;
        }

        buffer[n] = '\0'; // Null-terminate received data
        printf("Received raw radar data: %s\n", buffer);

        // Process the radar data
        ProcessRadarData(buffer, state);

        // Display parsed tracks
        pthread_mutex_lock(&state->state_mutex);
        for (uint32_t i = 0; i < state->num_tracks; i++) {
            RadarTrack *track = &state->tracks[i];
            if (track->is_valid) {
                printf("Track ID: %u | Lat: %.6f | Lon: %.6f | Alt: %.2f m | Vel: %.2f m/s | Head: %.2f° | Conf: %u%%\n",
                       track->track_id, track->latitude, track->longitude,
                       track->altitude, track->velocity, track->heading,
                       track->confidence);
            }
        }
        pthread_mutex_unlock(&state->state_mutex);
    }
}
