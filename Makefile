# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread

# Target executable
TARGET = daa_processor

# Source files
SRCS = main.c daa_processor.c drvadsb.c ecoflight_radar.c

# Header files
HDRS = daa_processor.h drvadsb.h ecoflight_radar.h

# Object files
OBJS = $(SRCS:.c=.o)

# Default rule
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Compile source files into object files
%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up the build
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
