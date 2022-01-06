all: build

build: $(patsubst source/%.cpp,objects/%.o, $(wildcard source/*.cpp))
	g++ $(wildcard objects/*.o) -o lsmarkov -O3

objects/%.o: source/%.cpp
	mkdir -p objects; g++ -O3 -I include -c -o $@ $< -std=c++14\
        -Wfatal-errors -pedantic -pedantic-errors -Wall -Wextra -Werror

clean:
	rm -f objects/*.o
