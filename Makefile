SOURCES:=$(wildcard src/*.cpp)

.PHONY: default all test install

default: all

all:
	g++ -o tinr $(SOURCES) -lncurses

test: all
	./tinr

install: all
	sudo mv ./tinr /usr/bin/tinr
