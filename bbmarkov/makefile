all: build

build: $(patsubst source/%.cpp,objects/%.o, $(wildcard source/*.cpp))
	g++ -O3 $(patsubst source/%.cpp,objects/%.o, $(wildcard source/*.cpp)) -o bbmarkov

objects/%.o: source/%.cpp
	mkdir -p objects; g++ -O3 -I include -c -o $@ $< -std=c++0x -Wfatal-errors

clean:
	rm -f objects/*.o;