platform ?= linux

sources := $(wildcard source/*.cpp)
compiled := $(patsubst %.cpp, build/$(platform)/%.o, $(notdir $(sources)))

compile_arguments := -c -g -std=c++20 -O3 -mavx -mavx2 -I source -I third_party
link_arguments :=

ifeq ($(platform), linux)
compiler = clang++
else ifeq ($(platform), windows)
compiler = x86_64-w64-mingw32-g++ -static
else
$(error valid platforms are windows and linux)
endif

all: build/$(platform)/downscale
.PHONY: all

clean:
	rm -vf build/linux/*
	rm -vf build/windows/*
.PHONY: clean

build/$(platform)/downscale: $(compiled)
	$(compiler) $(link_arguments) $^ -o $@

build/$(platform)/%.o: source/%.cpp
	$(compiler) $(compile_arguments) $< -o $@

