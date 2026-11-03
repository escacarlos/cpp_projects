################################################################################
# Declare some Makefile variables
################################################################################
CC = g++
LANG_STD = -std=c++17
COMPILER_FLAGS = -Wall -Wfatal-errors
SRC_FILES = ./lib/SDL2_gfx/*.c \
			./src/*.cpp \
			./src/Physics/*.cpp \

LINKER_FLAGS = -lSDL2main -lSDL2 -lSDL2_image #-lmingw32 
OBJ_NAME = physicengine

################################################################################
# Declare some Makefile rules
################################################################################
build:
	$(CC) $(COMPILER_FLAGS) $(LANG_STD) $(INCLUDE_PATH) $(SRC_FILES) $(LINKER_FLAGS) -o $(OBJ_NAME)

run:
	./$(OBJ_NAME)

clean:
	rm $(OBJ_NAME)
