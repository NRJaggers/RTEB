#PHONY targets to execute recipe 
.PHONY: all clean

#compiler and linker options
CC = gcc
CXX = g++
OUT = out.o
LDFLAGS = -g
CFLAGS = -Werror

OPENCV = `pkg-config opencv4 --cflags --libs`

#executable dependecies 
DEPS = ${wildcard *.h}
SRCS = ${wildcard *.cpp}

#default build target
all: default

default:
	${CXX} ${CFLAGS} ${LDFLAGS} ${SRCS} ${DEPS} -o ${OUT} ${OPENCV}
clean:
	rm -f ${OUT}
