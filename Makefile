OBJS = main.o Loader.o

all: loader

loader: $(OBJS) Loader.h
	g++ --std=c++0x $(OBJS) -o $@ -lcurl -lpthread

%.o: %.cpp
	g++ --std=c++0x -c -O2 -Wall $< -o $@

clean:
	rm -f *.o loader

.PHONY: clean

