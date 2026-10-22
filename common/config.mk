# Shared build settings, included by every weekNN/Makefile.
# Override on the command line, e.g.:  make CC=gcc-14 OPT=-O3

OPT      ?= -O2
CFLAGS   ?= $(OPT) -Wall -Wextra -std=gnu11 -I../common
CXXFLAGS ?= $(OPT) -Wall -Wextra -std=c++17 -I../common
LDLIBS_M := -lm

MPICC  ?= mpicc
NVCC   ?= nvcc
# Use e.g. NVCCFLAGS="-O2 -arch=native" on your own NVIDIA machine.
NVCCFLAGS ?= -O2 -I../common

UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
  # Apple clang needs libomp:  brew install libomp
  LIBOMP   := $(shell brew --prefix libomp 2>/dev/null)
  OMPFLAGS := -Xpreprocessor -fopenmp -I$(LIBOMP)/include
  OMPLIBS  := -L$(LIBOMP)/lib -lomp
else
  OMPFLAGS := -fopenmp
  OMPLIBS  :=
endif
