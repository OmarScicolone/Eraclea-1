CC     = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread \
         -Icore -Isensor -Idata -Iplatform -Icomm -Iground

LDFLAGS = -pthread

OBC_TARGET    = eraclea_obc
GROUND_TARGET = eraclea_ground

OBC_SRCS = \
    obc_main.c \
    core/system.c \
    comm/tc_handler.c \
    comm/tm_manager.c \
    comm/link.c \
    sensor/sensor.c \
    data/buffer.c \
    platform/platform.c

GROUND_SRCS = \
    ground_main.c \
    comm/link.c \
    ground/ground_sim.c

OBC_OBJS    = $(OBC_SRCS:.c=.o)
GROUND_OBJS = $(GROUND_SRCS:.c=.o)

all: $(OBC_TARGET) $(GROUND_TARGET)

$(OBC_TARGET): $(OBC_OBJS)
	$(CC) $(OBC_OBJS) -o $@ $(LDFLAGS)

$(GROUND_TARGET): $(GROUND_OBJS)
	$(CC) $(GROUND_OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBC_OBJS) $(GROUND_OBJS) $(OBC_TARGET) $(GROUND_TARGET)

rebuild: clean all
