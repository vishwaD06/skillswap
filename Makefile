# Makefile for SkillSwap (FODS in C)
CC = gcc
CFLAGS = -Wall -Wextra -O2 -Ibackend

# Detect Windows OS
ifeq ($(OS),Windows_NT)
    LDFLAGS = -lws2_32
    EXT = .exe
    RM = del /Q /F
else
    LDFLAGS = 
    EXT = 
    RM = rm -f
endif

BACKEND_SRCS = backend/student.c \
               backend/skill.c \
               backend/linked_list.c \
               backend/hash_table.c \
               backend/queue.c \
               backend/stack.c \
               backend/graph.c \
               backend/bst.c \
               backend/heap.c \
               backend/trie.c \
               backend/dynarray.c \
               backend/sorting.c \
               backend/matching.c \
               backend/storage.c

all: server test cli

server: $(BACKEND_SRCS) backend/server.c backend/main.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o skillswap_server$(EXT)
	@echo [BUILD SUCCESS] skillswap_server$(EXT) created!

test: $(BACKEND_SRCS) tests/test_data_structures.c
	$(CC) $(CFLAGS) $^ -o tests/test_data_structures$(EXT)
	@echo [BUILD SUCCESS] tests/test_data_structures$(EXT) created!
	tests/test_data_structures$(EXT)

cli: $(BACKEND_SRCS) cli/skillswap_cli.c
	$(CC) $(CFLAGS) $^ -o cli/skillswap_cli$(EXT)
	@echo [BUILD SUCCESS] cli/skillswap_cli$(EXT) created!

clean:
ifeq ($(OS),Windows_NT)
	-del /Q /F skillswap_server.exe tests\test_data_structures.exe cli\skillswap_cli.exe
else
	-rm -f skillswap_server tests/test_data_structures cli/skillswap_cli
endif
