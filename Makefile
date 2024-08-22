# Executable name
TARGET = minishell

# Compliler
CC = gcc

# Compliler flags
CFLAGS = -Wall -g

# Source file
SRCS = main.c

# File object (generated from source files)
OBJS = $(SRCS:.c=.o)

# Rule to compile the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile the object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to clean the generated files
clean:
	rm -f $(OBJS) $(TARGET)

# Rule to clean and recompile from scratch
re: clean $(TARGET)

# Indicates that these rules do not correspond to real files
.PHONY: all clean re
