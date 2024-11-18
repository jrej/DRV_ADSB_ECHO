#include "drvadsb.h"
#include <signal.h>

DAA_RDR_states states;

void SignalHandler(int signal) {
    printf("Interrupt signal received. Shutting down...\n");
    states.run_flag = 0;
}

int main(int argc, char **argv) {
    signal(SIGINT, SignalHandler);

    InitState(&states);
    ProcessIncomingMessages(&states);

    pthread_mutex_destroy(&states.mutex);
    printf("Program terminated gracefully.\n");
    return 0;
}
