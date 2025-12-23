# -------- Toolchain --------
CC      := gcc

# -------- Files --------
SRC     := main.c hash.c repository.c utl.c object.c compress.c
BIN     := a.out

# -------- Flags --------
CFLAGS  := -std=gnu11 -Wall -Wno-unused-variable -Wno-unused-function
LIBS    := -lcrypto -lm -lz

# -------- Build modes --------
DEBUG_CFLAGS   := -g -DLOG_ENABLE_DEBUG -DLOG_LEVEL=LOG_DEBUG
RELEASE_CFLAGS := -O2 -DLOG_LEVEL=LOG_INFO

# -------- Targets --------
.PHONY: build debug nurepo valgrind clean

# Release build (default)
build:
	rm -f $(BIN)
	$(CC) $(CFLAGS) $(RELEASE_CFLAGS) $(SRC) $(LIBS) -o $(BIN)

# Debug build (DEBUG enabled)
debug:
	rm -f $(BIN)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) $(SRC) $(LIBS) -o $(BIN)

# Run release build
nurepo: build
	./$(BIN)

# Valgrind run (always debug)
valgrind:
	rm -f $(BIN)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) $(SRC) $(LIBS) -o $(BIN)
	valgrind --tool=memcheck --leak-check=full ./$(BIN)

# Clean
clean:
	rm -f $(BIN)
