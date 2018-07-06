TARGET=code
SRCS = main.cpp

OBJS = $(SRCS:.c=.o)

$(TARGET):$(OBJS)
	g++ main.cpp -O2 -o code -std=c++17

clean:
	rm -rf code
