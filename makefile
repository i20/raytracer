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

SDIR := sources
HDIR := headers
ODIR := objects
LDIR := libraries
LJSONDIR := $(LDIR)/libjson-7.6.1

#########################################################################

# Up to date version of GCC installed via homebrew
# This one is perfectly working for CPU build but is not supported by NVCC for GPU build
CC := /usr/local/bin/g++-6

# Prefer default Xcode Clang for GPU as it is supported by NVCC to homogeneize CPU and GPU build
# Default g++ => clang++ + some essential configs
# CC := g++

# http://stackoverflow.com/questions/11344965/how-to-include-static-library-in-makefile
# https://www.quora.com/When-using-OpenMP-with-gcc-g++-clang-why-the-flags-prefix-is-f-instead-of-lopenmp
# http://www.learncpp.com/cpp-tutorial/a1-static-and-dynamic-libraries/

# INCLUDE PATHS
# -I adds directory for #include
# -F adds directory for framework #include -framework (specific Mac variant of -l, searches for <name>.framework/<name>)

# LINKING PATHS
# -L adds directory for archive/static (.lib/.a) libraries linking with -l (searches for lib<name>.a)
# -l adds link with a static library in -L path
# -framework adds link with a framework in -F
# -rpath adds directory for runtime/dynamic/shared (.dll/.so) libraries (cf DYLD ...)

# Use -Xlinker to pass linker options directly and bypass gcc / clang / nvcc own logic
# That way building with clang or nvcc should work the same way

# Cuda does not support C++14 officially
FLAGS := -Wall -pedantic -O3 -std=c++14 -fopenmp\
-F"$(LDIR)"\
-I"$(LJSONDIR)"

LDFLAGS := -L"$(LJSONDIR)"\
-ljson\
-Xlinker -rpath -Xlinker "$(LDIR)"\
-framework SDL2\
-framework SDL2_ttf

EXEC := raytracer
MAIN := main

#########################################################################

CUDA_INSTALL_PATH := /usr/local/cuda
NVCC := $(CUDA_INSTALL_PATH)/bin/nvcc
# https://devtalk.nvidia.com/default/topic/418143/dyld-library-not-loaded-rpath-libcudart-dylib-/
# -stdlib=libc++ : https://stackoverflow.com/questions/40301619/unordered-map-file-not-found-error-when-compiling-with-xcode-7-3-1
# https://groups.google.com/forum/#!msg/thrust-users/ERLq6mGEQAs/0cAcWcUbUHMJ
# https://devtalk.nvidia.com/default/topic/741707/cuda-setup-and-installation/cuda-6-mac-os-10-9-libstdc-still-required-/
NVCC_FLAGS := $(FLAGS) -stdlib=libstdc++ -D _GPU -Xlinker -rpath -Xlinker "$(CUDA_INSTALL_PATH)/lib"
NVCC_EXEC := gpu-raytracer

#########################################################################

# detecte automatiquement la liste des fichiers sources (main excepté)
HEADERS := $(wildcard $(HDIR)/*.hpp)
CLASSES := $(HEADERS:$(HDIR)/%.hpp=%)

#########################################################################

.PHONY: all clean reset

all: $(EXEC)# $(NVCC_EXEC)

$(EXEC): $(ODIR)/$(MAIN).o $(CLASSES:%=$(ODIR)/%.o) $(LJSONDIR)/libjson.a
	$(CC) -o $@ $(ODIR)/$(MAIN).o $(CLASSES:%=$(ODIR)/%.o) $(FLAGS) $(LDFLAGS)

# Hardcoded scenes are part of prerequisites
$(ODIR)/$(MAIN).o: $(SDIR)/$(MAIN).cpp $(wildcard scenes/*.cpp)
	$(CC) -o $@ -c $< $(FLAGS)

$(ODIR)/%.o: $(SDIR)/%.cpp $(HDIR)/%.hpp
	$(CC) -o $@ -c $< $(FLAGS)

# https://www.gnu.org/software/make/manual/make.html#Recursion
$(LJSONDIR)/libjson.a:
	$(MAKE) -C $(LJSONDIR)

#########################################################################

# Make sure main.cpp is after other .cpp in dependencies list to append it at the end of the .cu
$(NVCC_EXEC): $(HEADERS) $(CLASSES:%=$(SDIR)/%.cpp) $(SDIR)/$(MAIN).cpp $(wildcard scenes/*.cpp)
# Recipe comments must not be prefixed by a tab otherwise it is taken as part of the recipe
# echo adds "\n" by default
# $$<var> is to use normal bash var, $(var) is for makefile ones
	@for file in $(HEADERS) $(CLASSES:%=$(SDIR)/%.cpp) $(SDIR)/$(MAIN).cpp; do\
		cat $$file;\
		echo;\
	done > $(SDIR)/$@.cu
	$(NVCC) -ccbin $(CC) $(SDIR)/$@.cu -o $@ $(NVCC_FLAGS:%=-Xcompiler %)

gpu-info: $(SDIR)/$@.cu
	$(NVCC) -ccbin $(CC) $< -o $@ $(NVCC_FLAGS:%=-Xcompiler %)

#########################################################################

# Removes intermediate compilation files
clean:
	rm -f $(ODIR)/*.o $(SDIR)/$(NVCC_EXEC).cu

# Removes all compilation traces
reset: clean
# gpu-info
	rm -f $(EXEC) $(NVCC_EXEC)
