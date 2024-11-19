#include "drvadsb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/******************************************************************************
 * Initialize the ADS-B Listener
 ******************************************************************************/
void InitADSBListener(int *sockfd, struct sockaddr_in *serveraddr) {
    // Create UDP socket
    *sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (*sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket to the ADS-B port
    memset(serveraddr, 0, sizeof(*serveraddr));
    serveraddr->sin_family = AF_INET;
    serveraddr->sin_addr.s_addr = INADDR_ANY;
    serveraddr->sin_port = htons(PORT_ADSB);

    if (bind(*sockfd, (const struct sockaddr *)serveraddr, sizeof(*serveraddr)) < 0) {
        perror("Bind failed");
        close(*sockfd);
        exit(EXIT_FAILURE);
    }

    printf("ADSB Listener initialized on port %d\n", PORT_ADSB);
}

/******************************************************************************
 * Parse Raw ADS-B Data
 ******************************************************************************/
void ProcessADSBMessage(const char *raw_data, ADSBMessage *message) {
    if (raw_data == NULL || message == NULL) {
        return;
    }

    // Mock parsing logic: Replace this with actual ADS-B decoding logic.
    sscanf(raw_data, "%x,%f,%f,%f,%f,%f",
           &message->icao_address,
           &message->latitude,
           &message->longitude,
           &message->altitude,
           &message->velocity,
           &message->heading);

    // Example: Set validity based on parsed values
    message->is_valid = (message->latitude != 0.0f && message->longitude != 0.0f);
}

/******************************************************************************
 * Receive ADS-B Packets
 ******************************************************************************/
void ReceiveADSBPackets(int sockfd) { 
    char buffer[BUFSIZE];
    struct sockaddr_in clientaddr;
    socklen_t addrlen = sizeof(clientaddr);
    ADSBMessage message;

    printf("Waiting for ADS-B packets...\n");

    while (1) {
        ssize_t n = recvfrom(sockfd, buffer, BUFSIZE, 0, (struct sockaddr *)&clientaddr, &addrlen);
        if (n < 0) {
            perror("recvfrom failed");
            continue;
        }

        buffer[n] = '\0'; // Null-terminate received data
        printf("Received raw data: %s\n", buffer);

        // Process the ADS-B message
        ProcessADSBMessage(buffer, &message);

        if (message.is_valid) {
            printf("Decoded ADS-B Message:\n");
            printf("  ICAO Address: %06X\n", message.icao_address);
            printf("  Latitude: %.6f\n", message.latitude);
            printf("  Longitude: %.6f\n", message.longitude);
            printf("  Altitude: %.2f ft\n", message.altitude);
            printf("  Velocity: %.2f knots\n", message.velocity);
            printf("  Heading: %.2f degrees\n", message.heading);
        } else {
            printf("Invalid ADS-B message received.\n");
        }
    }
}
