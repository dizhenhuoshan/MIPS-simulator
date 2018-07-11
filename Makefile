TARGET=code
SRCS = main.cpp

OBJS = $(SRCS:.c=.o)

$(TARGET):$(OBJS)
	g++ main.cpp -O3 -o code -std=c++17 -march=corei7-avx

clean:
	rm -rf code
