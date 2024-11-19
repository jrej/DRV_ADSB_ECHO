#include "drvadsb.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int sockfd;

void SignalHandler(int signal) {
    printf("Interrupt signal received. Shutting down...\n");
    close(sockfd);
    exit(0);
}

int main() {
    struct sockaddr_in serveraddr;

    // Handle termination signals
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    // Initialize ADS-B listener
    InitADSBListener(&sockfd, &serveraddr);

    // Start receiving packets
    ReceiveADSBPackets(sockfd); 

    return 0;
}
