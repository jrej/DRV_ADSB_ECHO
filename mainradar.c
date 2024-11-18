#include "ecoflight_radar.h"
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

RadarState radar_state;

void SignalHandler(int signal) {
    printf("Signal received. Shutting down radar driver...\n");
    close(radar_state.sockfd);
    pthread_mutex_destroy(&radar_state.state_mutex);
    exit(0);
}

int main() {
    signal(SIGINT, SignalHandler);

    // Initialize radar driver
    InitRadarDriver(&radar_state);

    // Start receiving packets
    ReceiveRadarPackets(&radar_state);

    return 0;
}
