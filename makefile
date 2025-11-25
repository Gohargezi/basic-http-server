# Compiler & Flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I./lib -I./src

# Folders
SRC_DIR = src
LIB_DIR = lib
WWW_DIR = www

# Output
TARGET = http-server

# Object files
OBJS = $(SRC_DIR)/http-server.o $(LIB_DIR)/strlib.o

# Default build target
all: $(TARGET)

# Build final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Build object files
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_DIR)/%.o: $(LIB_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run server
run: $(TARGET)
	./$(TARGET)
# Run tests (ensure server is built first)
test: $(TARGET)
	cd test && \
	echo "Building tests..." && \
	gcc -std=c11 -Wall -Wextra -I../lib -I../src test_server.c unity.c -o test_server && \
	echo "Running tests..." && \
	./test_server

# Clean compiled files
clean:
	rm -f $(SRC_DIR)/*.o $(LIB_DIR)/*.o $(TARGET)

# Phony targets
.PHONY: all clean run test
