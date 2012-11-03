CXX=clang++
CXXFLAGS=-Wall -Wextra -pedantic -Werror -std=c++11 -g

all: varvector

clean:
	$(RM) varvector
