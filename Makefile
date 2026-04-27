# sysctrace - Lightweight Syscall Tracer
# Computer Architecture Project

CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -std=gnu11 -Iinclude
LDFLAGS :=

# Debug build flags
DEBUG_FLAGS := -g -O0 -DDEBUG

SRC_DIR := src
OBJ_DIR := build
BIN     := sysctrace

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

.PHONY: all clean debug run help

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "[build] $(BIN) ready"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean all

run: $(BIN)
	./$(BIN)

clean:
	@rm -rf $(OBJ_DIR) $(BIN)
	@echo "[clean] done"

help:
	@echo "Targets:"
	@echo "  make          - build release version"
	@echo "  make debug    - build with -g -O0"
	@echo "  make run      - build and run"
	@echo "  make clean    - remove build artifacts"