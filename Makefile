CC = gcc
VERSION = -std=c11
FLAGS = -Wall
SRC := $(wildcard ./*.c) $(wildcard ./glad/src/*.c)
INCLUDEPATH = -I./ -I./glad/include/
LIBPATH = -L./
LIBS = -lm -lpthread -ldl -lGL -lglfw
TARGET = ./main

.SILENT:
.PHONY: all just clean

all:
	${CC} ${VERSION} ${FLAGS} ${INCLUDEPATH} ${LIBPATH} ${SRC} ${LIBS} -o ${TARGET}
	${TARGET}

just:
	${CC} ${VERSION} ${FLAGS} ${INCLUDEPATH} ${LIBPATH} ${SRC} ${LIBS} -o ${TARGET}

clean:
	$(RM) ${TARGET}