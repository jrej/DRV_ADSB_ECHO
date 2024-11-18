#ifndef ECOFLIGHT_RADAR_H
#define ECOFLIGHT_RADAR_H

/******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

/******************************************************************************
 * Constants and Macros
 ******************************************************************************/
#define RADAR_BUFFER_SIZE 2048
#define RADAR_PORT 55000
#define MAX_TRACKS 256

/******************************************************************************
 * Data Structures
 ******************************************************************************/
typedef struct RadarTrack {
    uint32_t track_id;       // Unique ID for the track
    float latitude;          // Latitude of the target
    float longitude;         // Longitude of the target
    float altitude;          // Altitude in meters
    float velocity;          // Velocity in m/s
    float heading;           // Heading in degrees
    uint8_t confidence;      // Confidence level (0-100)
    uint8_t is_valid;        // Track validity flag
} RadarTrack;

typedef struct RadarState {
    int sockfd;                          // Socket file descriptor
    struct sockaddr_in server_addr;      // Server address
    RadarTrack tracks[MAX_TRACKS];       // Array of detected tracks
    uint32_t num_tracks;                 // Number of active tracks
    pthread_mutex_t state_mutex;         // Mutex for thread safety
} RadarState;

/******************************************************************************
 * Function Prototypes
 ******************************************************************************/
void InitRadarDriver(RadarState *state);
void ProcessRadarData(const char *raw_data, RadarState *state);
void ReceiveRadarPackets(RadarState *state);

#endif // ECOFLIGHT_RADAR_H
