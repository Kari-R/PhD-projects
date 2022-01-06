all: build

build: $(patsubst source/%.cpp,objects/%.o, $(wildcard source/*.cpp))
	g++ $(wildcard objects/*.o) -o scorer

objects/%.o: source/%.cpp
	mkdir -p objects; g++ -O3 -I include -I lib -c -o $@ $< -std=c++14 -Wfatal-errors -w

clean:
	rm -f objects/*.o
