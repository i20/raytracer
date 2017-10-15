#########################################################################
# COMMON VARS:
#
# $@ - target name
# $< - first dependency
# $^ - dependencies list
# $? - liste des dependances plus recentes que la cible
# $* - basename without extension
#
# VARS ASSIGNMENT:
#
# var = val - val is expanded at call time
# var := val - val is expanded at assignment time
# var ?= val - var is assigned val only if not already defined
#
# @instruction : quiet, no print on stdout
#########################################################################

#CUDA_INSTALL_PATH := /usr/local/cuda

# Up to date version of GCC installed via homebrew, supports C++14
CC := /usr/local/bin/g++-6

SDIR := sources
HDIR := headers
ODIR := objects
LDIR := libraries

EXEC := raytracer
MAIN := main

# http://stackoverflow.com/questions/11344965/how-to-include-static-library-in-makefile
# https://www.quora.com/When-using-OpenMP-with-gcc-g++-clang-why-the-flags-prefix-is-f-instead-of-lopenmp
# -L adds directory for library linking -l (searches for lib<name>.a)
# -I adds directory for #include
# -F adds directory for -framework (specific Mac variant of -l, searches for <name>.framework/<name>)
#    base is /Library/Frameworks and /Users/<???>/Library/Framework
# Type "gcc -xc++ -E -v -" for more informations
LIBS := -L$(LDIR)/libjson-7.6.1 -ljson -fopenmp -F$(LDIR) -framework SDL2 -framework SDL2_ttf -Xlinker -rpath -Xlinker $(LDIR) -Xlinker -rpath -Xlinker $(LDIR)/libjson-7.6.1
FLAGS := -Wall -pedantic -O3

# http://stackoverflow.com/questions/39865367/warning-section-textcoal-nt-is-deprecate-since-updating-to-mac-osx-sierra
FILTER_OUTPUT := | grep -v -e '^/var/folders/*' -e '^[[:space:]]*\.section' -e '^[[:space:]]*\^[[:space:]]*~*'

# detecte automatiquement la liste des fichiers sources (main excepté)
SRCS := $(filter-out $(SDIR)/$(MAIN).cpp,$(wildcard $(SDIR)/*.cpp))
# determine la liste des fichiers objets qui devront etre crees a partir de la liste des sources
# cf http://sunsite.ualberta.ca/Documentation/Gnu/make-3.79/html_chapter/make_8.html
OBJS := $(SRCS:$(SDIR)/%.cpp=$(ODIR)/%.o)

.PHONY: all clean reset cuda testmatrix

all: $(EXEC)

$(EXEC): $(ODIR)/$(MAIN).o $(OBJS)
	$(CC) -o $@ $^ $(LIBS)

# Hardcoded scenes are part of prerequisites
$(ODIR)/$(MAIN).o: $(SDIR)/$(MAIN).cpp $(wildcard scenes/*.cpp)
	$(CC) -o $@ -c $< $(FLAGS) $(LIBS)

$(ODIR)/%.o: $(SDIR)/%.cpp $(HDIR)/%.hpp
	$(CC) -o $@ -c $< $(FLAGS) $(LIBS)

clean:
	@rm -f $(ODIR)/*.o

reset: clean
	@rm -f $(EXEC)

testmatrix:
	$(CC) -o ./tests/testmatrix ./tests/TestMatrix.cpp ./sources/Matrix.cpp ./sources/Vector.cpp ./sources/Point.cpp ./sources/Ray.cpp

# Temp target to build cuda test
# https://devtalk.nvidia.com/default/topic/418143/dyld-library-not-loaded-rpath-libcudart-dylib-/
#cuda:
#	$(CUDA_INSTALL_PATH)/bin/nvcc test_cuda.cu -o test_cuda -Xlinker -rpath,$(CUDA_INSTALL_PATH)/lib
