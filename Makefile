# Compiler instellingen
CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS =
LDLIBS =

# Directory structuur
SRC_DIR = ~/cweb/src
BUILD_DIR = ~/cweb/build
TARGET = $(BUILD_DIR)/cweb

# Bestanden zoeken
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Default regel
all: $(BUILD_DIR) $(TARGET)

# Uitvoerbaar bestand maken
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Build directory aanmaken
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# C bestanden compileren naar object bestanden in build directory
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Opruimen
clean:
	rm -rf $(BUILD_DIR)

# Hercompileer alles
rebuild: clean all

# Hulplijn tonen
help:
	@echo "Beschikbare commando's:"
	@echo "  make all     - Bouw het programma (default)"
	@echo "  make         - Zelfde als 'make all'"
	@echo "  make clean   - Verwijder build directory"
	@echo "  make rebuild - Herbouw volledig"
	@echo "  make help    - Toon dit help bericht"
	@echo ""
	@echo "Directory structuur:"
	@echo "  Broncode:   $(SRC_DIR)/"
	@echo "  Output:     $(BUILD_DIR)/"

.PHONY: all clean rebuild help
