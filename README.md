# DRV_ADSB_ECHO
README: DAA Processor with ADS-B and EcoFlight Radar Integration

This project implements a Detect-And-Avoid (DAA) system that merges data from an ADS-B driver and an EcoFlight Radar driver into a unified circular buffer. The processed data is accessible for other systems, ensuring smooth integration of multiple data streams for collision detection and avoidance.
Features
1. ADS-B Driver

    Collects and decodes ADS-B data.
    Extracts information such as ICAO address, latitude, longitude, altitude, velocity, and heading.
    Outputs structured data into the DAA system.

2. EcoFlight Radar Driver

    Collects and decodes radar track data.
    Extracts track details like ID, latitude, longitude, altitude, velocity, heading, and confidence level.
    Outputs structured radar data into the DAA system.

3. DAA Processor

    Merges data from ADS-B and EcoFlight Radar.
    Uses a thread-safe circular buffer to store merged tracks.
    Exposes a clean interface for retrieving processed tracks.
    Includes synchronization mechanisms for concurrent data access.

Project Structure

.
├── daa_processor.h         # Header file for the DAA processing system
├── daa_processor.c         # Implementation of the DAA processor
├── drvadsb.h               # Header file for the ADS-B driver
├── drvadsb.c               # Implementation of the ADS-B driver
├── ecoflight_radar.h       # Header file for the EcoFlight Radar driver
├── ecoflight_radar.c       # Implementation of the EcoFlight Radar driver
├── main.c                  # Main entry point of the program
└── Makefile                # Makefile for building the project

Build Instructions
Prerequisites

    A C compiler (e.g., GCC)
    pthread library

Steps

    Clone the repository or download the source files.

    Navigate to the project directory.

    Compile the project using the following command:

gcc -o daa_processor main.c daa_processor.c drvadsb.c ecoflight_radar.c -lpthread

Run the program:

    ./daa_processor

How It Works
ADS-B Driver

    Listens for incoming ADS-B data over UDP.
    Decodes raw ADS-B messages into structured ADSBMessage objects.
    Converts units (e.g., altitude in feet to meters, velocity in knots to m/s).
    Validates data and feeds it into the DAA processor.

EcoFlight Radar Driver

    Listens for radar data over UDP.
    Decodes raw radar data into structured RadarTrack objects.
    Validates data and feeds it into the DAA processor.

DAA Processor

    Merging:
        Accepts data from both ADS-B and radar sources.
        Combines the data into a unified DAAMergedTrack format.

    Circular Buffer:
        Stores the merged data using a circular buffer.
        Supports thread-safe operations using pthread mutexes and condition variables.

    Data Access:
        Provides a GetMergedData function to retrieve the next available track.
        Signals when new data is added to the buffer.

Example Output

When the system runs, it displays the merged track data:

ADSB Driver initialized on port 50000
EcoFlight Radar Driver initialized on port 55000
DAA Processor started...
Waiting for radar packets...
Merged Track [Source: ADS-B | ID: ABC123 | Lat: 37.774900 | Lon: -122.419400 | Alt: 9144.00 m | Vel: 231.48 m/s | Head: 90.00° | Conf: 100%]
Merged Track [Source: Radar | ID: 1001 | Lat: 37.775000 | Lon: -122.419500 | Alt: 9100.00 m | Vel: 250.00 m/s | Head: 85.00° | Conf: 95%]