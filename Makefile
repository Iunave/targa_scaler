sources := $(wildcard source/*.cpp)
compiled := $(patsubst %.cpp, build/%.o, $(notdir $(sources)))

compile_arguments := -c -g -std=c++20 -O3 -march=native -I source -I third_party
link_arguments := -lfmt

all: build/downscale
.PHONY: all

clean:
	rm -vf build/*
.PHONY: clean

build/downscale: $(compiled)
	clang++ $(link_arguments) $^ -o $@

build/%.o: source/%.cpp
	clang++ $(compile_arguments) $< -o $@

