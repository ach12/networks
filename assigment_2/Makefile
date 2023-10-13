CXX = g++
CXXFLAGS = -g -Wall

all: rsshell amp

## Common:
tcp-utils.o: tcp-utils.h tcp-utils.cc
	$(CXX) $(CXXFLAGS) -c -o tcp-utils.o tcp-utils.cc

## Client:
shell.o: tcp-utils.h shell.cc
	$(CXX) $(CXXFLAGS) -c -o shell.o shell.cc

rsshell: shell.o tcp-utils.o
	$(CXX) $(CXXFLAGS) -o rsshell shell.o tcp-utils.o

amp: amp.cc
	$(CXX) $(CXXFLAGS) -o amp amp.cc

clean:
	rm -f rsshell amp *~ *.o *.bak core \#*
