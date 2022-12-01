#PHONY Targets to Execute Reciple
.PHONY: all clean

#compiler and linker options
CC = gcc
CXX = g++
OUT = sobel
LDFLAGS = -g
CFLAGS =  -flax-vector-conversions -Ofast
OFLAGS = -finline-functions -floop-block -fselective-scheduling -floop-parallelize-all -funroll-loops
OPENCV = `pkg-config opencv4 --cflags --libs`

#executable dependencies
DEPS = ${wildcard *.h}
SRCS = ${wildcard *.cpp}

#default build target
all: default

default:
	${CXX} ${CFLAGS} ${LDFLAGS} ${SRCS} ${DEPS} -o ${OUT} ${OPENCV}
JoshAlab:
	${CXX} ${CFLAGS} ${LDFLAGS} lab5.cpp ${DEPS} -o ${OUT} ${OPENCV}
dispIm: #Force this make statement by typing 'make dispIm'
	g++ DisplayImage.cpp -o sobel 'pkg-config opencv4 --cflags --libs'
opt:
	${CXX} ${CFLAGS} ${OFLAGS} ${LDFLAGS} ${SRCS} ${DEPS} -o ${OUT} ${OPENCV}
clean:
	rm -f ${OUT}
