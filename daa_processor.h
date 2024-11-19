#ifndef DAA_PROCESSOR_H
#define DAA_PROCESSOR_H

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "drvadsb.h"
#include "ecoflight_radar.h"
#include <pthread.h>

/******************************************************************************
 * Constants and Macros
 ******************************************************************************/
#define DAA_BUFFER_SIZE 256

/******************************************************************************
 * Data Structures
 ******************************************************************************/
typedef struct DAAMergedTrack {
    uint32_t source;      // 0 = ADS-B, 1 = Radar
    uint32_t track_id;    // Unique ID for the track
    float latitude;       // Latitude in decimal degrees
    float longitude;      // Longitude in decimal degrees
    float altitude;       // Altitude in meters
    float velocity;       // Velocity in m/s
    float heading;        // Heading in degrees
    uint8_t confidence;   // Confidence level (0-100)
    uint8_t is_valid;     // Track validity
} DAAMergedTrack;

typedef struct DAACircularBuffer {
    DAAMergedTrack buffer[DAA_BUFFER_SIZE]; // Circular buffer for merged tracks
    int head;                               // Index of the oldest element
    int tail;                               // Index of the next free slot
    int count;                              // Number of elements in the buffer
    pthread_mutex_t mutex;                  // Mutex for thread-safe access
    pthread_cond_t cond;                    // Condition variable for signaling
} DAACircularBuffer;

/******************************************************************************
 * Function Prototypes
 ******************************************************************************/
void InitDAAProcessor(DAACircularBuffer *cbuf);
void MergeADSBData(const ADSBMessage *adsb_msg, DAACircularBuffer *cbuf);
void MergeRadarData(const RadarTrack *radar_track, DAACircularBuffer *cbuf);
int GetMergedData(DAACircularBuffer *cbuf, DAAMergedTrack *output);

#endif // DAA_PROCESSOR_H
