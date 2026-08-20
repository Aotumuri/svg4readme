CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -std=c11
SRC_DIR := src
BUILD_DIR := build
OUTPUT_DIR := output

.PHONY: all waves fire forest clean

all: $(BUILD_DIR)/waves_svg $(BUILD_DIR)/fire_svg $(BUILD_DIR)/forest_svg

$(BUILD_DIR):
	mkdir -p $@

$(OUTPUT_DIR):
	mkdir -p $@

$(BUILD_DIR)/waves_svg: $(SRC_DIR)/waves_svg.c $(SRC_DIR)/config.c $(SRC_DIR)/config.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/waves_svg.c $(SRC_DIR)/config.c -lm -o $@

$(BUILD_DIR)/fire_svg: $(SRC_DIR)/fire_svg.c $(SRC_DIR)/config.c $(SRC_DIR)/config.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/fire_svg.c $(SRC_DIR)/config.c -lm -o $@

$(BUILD_DIR)/forest_svg: $(SRC_DIR)/forest_svg.c $(SRC_DIR)/config.c $(SRC_DIR)/config.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/forest_svg.c $(SRC_DIR)/config.c -o $@

waves: $(BUILD_DIR)/waves_svg | $(OUTPUT_DIR)
	./$(BUILD_DIR)/waves_svg > $(OUTPUT_DIR)/waves.svg

fire: $(BUILD_DIR)/fire_svg | $(OUTPUT_DIR)
	./$(BUILD_DIR)/fire_svg > $(OUTPUT_DIR)/fire.svg

forest: $(BUILD_DIR)/forest_svg | $(OUTPUT_DIR)
	./$(BUILD_DIR)/forest_svg > $(OUTPUT_DIR)/forest.svg

clean:
	rm -rf $(BUILD_DIR) $(OUTPUT_DIR)
