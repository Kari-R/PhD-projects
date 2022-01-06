all: build

build: $(patsubst source/%.cpp,objects/%.o, $(wildcard source/*.cpp))
	g++ $(wildcard objects/*.o) -o search -O3

objects/%.o: source/%.cpp
	mkdir -p objects; g++ -O3 -I include -c -o $@ $< -std=c++11 -Wfatal-errors -w

clean:
	rm -f objects/*.o
