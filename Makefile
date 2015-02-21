CXX=clang++
CXXFLAGS=-Wall -Wextra -pedantic -Werror -g -std=c++11

all: varvector

clean:
	$(RM) varvector
