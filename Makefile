# FFmpeg Audio Utils Library Makefile
# Compatible with Makefile.libs structure

CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -fPIC -std=c99
LDFLAGS = -shared
TARGET ?= ../../libs/libkylffmpeg.so

# Source files
SOURCES = ffmpeg_utils.c libkylffmpeg.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = ffmpeg_utils.h

# Check for ffmpeg
HAS_FFMPEG := $(shell command -v ffmpeg >/dev/null 2>&1 && echo yes || echo no)
HAS_FFPROBE := $(shell command -v ffprobe >/dev/null 2>&1 && echo yes || echo no)

# Installation directories
PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib
INCLUDEDIR = $(PREFIX)/include

.PHONY: all clean install uninstall test deps check-deps

all: check-deps $(TARGET)

# Install dependencies
deps:
	@echo "Installing FFmpeg dependencies..."
	@echo "Run: sudo apt-get install ffmpeg"
	@which apt-get > /dev/null 2>&1 && sudo apt-get install -y ffmpeg || echo "Please install ffmpeg manually"

# Check for dependencies
check-deps:
ifeq ($(HAS_FFMPEG),no)
	@echo "Error: ffmpeg not found."
	@echo "Install with: sudo apt-get install ffmpeg"
	@echo "Or run: make -C additional_libs/ffmpeg deps"
	@exit 1
endif
ifeq ($(HAS_FFPROBE),no)
	@echo "Error: ffprobe not found."
	@echo "Install with: sudo apt-get install ffmpeg"
	@echo "Or run: make -C additional_libs/ffmpeg deps"
	@exit 1
endif

# Build shared library
$(TARGET): $(OBJECTS)
	mkdir -p $(dir $(TARGET))
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)

# Compile object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Install library and headers
install: $(TARGET)
	install -d $(LIBDIR)
	install -d $(INCLUDEDIR)
	install -m 644 $(TARGET) $(LIBDIR)
	install -m 644 $(HEADERS) $(INCLUDEDIR)
	ldconfig

# Uninstall library and headers
uninstall:
	rm -f $(LIBDIR)/libkylffmpeg.so
	rm -f $(INCLUDEDIR)/ffmpeg_utils.h
	ldconfig

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: all

# Example usage
help:
	@echo "FFmpeg Audio Utils Library"
	@echo "=========================="
	@echo ""
	@echo "Available targets:"
	@echo "  make all      - Build the library (checks for ffmpeg)"
	@echo "  make deps     - Install ffmpeg dependencies"
	@echo "  make clean    - Clean build files"
	@echo "  make install  - Install library system-wide"
	@echo "  make debug    - Build with debug symbols"
	@echo ""
	@echo "Requirements:"
	@echo "  - ffmpeg (command-line tool)"
	@echo "  - ffprobe (command-line tool)"
