cc = g++
pram = code
deps = main.cpp

$(pram): $(deps)
	g++ -o $(pram) $(deps) -std=c++17

%.o: %.c $(deps)
	$(cc) -c $< -o $@

clean:
	rm -rf $(obj) $(pram)
