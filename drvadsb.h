#ifndef DRV_ADSB_H
#define DRV_ADSB_H

/******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

/******************************************************************************
 * Constants and Macros
 ******************************************************************************/
#define BUFSIZE 1024
#define NUM_IN_FDS 2
#define PORT_INPUT 50000
#define PORT_OUTPUT 50001
#define PORT_TRM 50080
#define MULTICAST_GROUP "224.0.0.94"

// Message IDs
#define STANAG_MSG_ID_DAA_PRIORITIZED_TRACK_REPORT 40001
#define STANAG_MSG_ID_DAA_SUBSYSTEM_STATUS_REPORT  40005
#define STANAG_MSG_ID_DAA_OPERATING_MODE_REPORT    40011
#define STANAG_MSG_ID_DAA_VERTICAL_RESOLUTION_ADVISORY 40015
#define STANAG_MSG_ID_DAA_HORIZONTAL_RESOLUTION_ADVISORY 40016

/******************************************************************************
 * Data Structures
 ******************************************************************************/
typedef struct DAA_RDR_states {
    int trackwriteIndex;
    int navwriteIndex;
    int trackreadIndex;
    int navreadIndex;
    int num_in_fds;
    uint8_t run_flag;
    struct pollfd fds[NUM_IN_FDS];
    char buffer[BUFSIZE];
    uint32_t instance_id;
    pthread_mutex_t mutex;
} DAA_RDR_states;

typedef struct StanagMessageHeader {
    int32_t instance_id_i32;
    int32_t message_id_i32;
    uint32_t message_len_ui32;
} StanagMessageHeader;

typedef struct Stanag40020DaaAtarIntruderDataReport {
    int track_id;
    double toa;
    float range;
    float altitude;
    uint8_t processed;
} Stanag40020DaaAtarIntruderDataReport;

/******************************************************************************
 * Function Prototypes
 ******************************************************************************/
void InitState(DAA_RDR_states *states);
uint32_t ComputeChecksum(const char *data, size_t len);
int VerifyChecksum(const char *data, size_t len);
void PackStanagMessageHeader(char *buffer, StanagMessageHeader *header);
void UnpackStanagMessageHeader(StanagMessageHeader *header, const char *buffer);
void HandlePrioritizedTrackReport(int sockfd, char *buf);
void HandleSubsystemStatusReport(int sockfd, char *buf);
void HandleOperatingModeReport(int sockfd, char *buf);
void HandleVerticalResolutionAdvisory(int sockfd, char *buf);
void HandleHorizontalResolutionAdvisory(int sockfd, char *buf);
void ProcessIncomingMessages(DAA_RDR_states *states);

#endif // DRV_ADSB_H
