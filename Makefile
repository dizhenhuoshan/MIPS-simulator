cc = g++
pram = code
deps = main.cpp

$(pram): $(deps)
	g++ -o $(pram) $(deps) -std=c++11

clean:
	rm -rf code
