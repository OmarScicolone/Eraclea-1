# Compiler
CC = gcc

# Flags
CFLAGS = -Wall -Wextra -std=c11 -pthread -Icore -Isensor -Idata -Iplatform -Icomm -Itasks
LDFLAGS = -pthread

# Nome eseguibile
TARGET = eraclea1

# Trova automaticamente tutti i .c
SRCS = $(wildcard *.c */*.c)

# Genera i .o
OBJS = $(SRCS:.c=.o)

# Default
all: $(TARGET)

# Link finale
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compilazione singoli file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Pulizia
clean:
	rm -f $(OBJS) $(TARGET)

# Rebuild completo
rebuild: clean all
