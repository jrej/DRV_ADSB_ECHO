#include "drvadsb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>

/******************************************************************************
 * Utility Functions
 ******************************************************************************/
uint32_t ComputeChecksum(const char *data, size_t len) {
    uint32_t crc = 0;
    for (size_t i = 0; i < len; i++) {
        crc += data[i];
    }
    return crc;
}

int VerifyChecksum(const char *data, size_t len) {
    uint32_t computed_crc = ComputeChecksum(data, len);
    uint32_t received_crc = *(uint32_t *)(data + len);
    return (computed_crc == received_crc);
}

void PackStanagMessageHeader(char *buffer, StanagMessageHeader *header) {
    memcpy(buffer, header, sizeof(StanagMessageHeader));
}

void UnpackStanagMessageHeader(StanagMessageHeader *header, const char *buffer) {
    memcpy(header, buffer, sizeof(StanagMessageHeader));
}

/******************************************************************************
 * Message Handlers
 ******************************************************************************/
void HandlePrioritizedTrackReport(int sockfd, char *buf) {
    Stanag40020DaaAtarIntruderDataReport report;
    StanagMessageHeader header;

    UnpackStanagMessageHeader(&header, buf);
    if (!VerifyChecksum(buf, header.message_len_ui32)) {
        printf("CRC verification failed for Message ID: %d\n", header.message_id_i32);
        return;
