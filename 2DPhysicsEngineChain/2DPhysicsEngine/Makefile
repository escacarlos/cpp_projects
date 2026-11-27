################################################################################
# Declare some Makefile variables
################################################################################
CC = g++
LANG_STD = -std=c++17
COMPILER_FLAGS = -Wall -Wfatal-errors
SRC_FILES = ./lib/SDL2_gfx/*.c \
			./src/*.cpp \
			./src/Physics/*.cpp \

LINKER_FLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image
ifeq ($(OS),Windows_NT)
OBJ_NAME = physicengine.exe
RUN_COMMAND = $(OBJ_NAME)
CLEAN_COMMAND = cmd /C del /Q $(OBJ_NAME)
else
OBJ_NAME = physicengine
RUN_COMMAND = ./$(OBJ_NAME)
CLEAN_COMMAND = rm -f $(OBJ_NAME)
endif

################################################################################
# Declare some Makefile rules
################################################################################
build:
	$(CC) $(COMPILER_FLAGS) $(LANG_STD) $(INCLUDE_PATH) $(SRC_FILES) $(LINKER_FLAGS) -o $(OBJ_NAME)

run:
	$(RUN_COMMAND)

clean:
	$(CLEAN_COMMAND)
