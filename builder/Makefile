MAIN = $(wildcard *.cpp)
PROGRAM = $(patsubst %.cpp, %, $(MAIN))

OPTIONS = -std=c++0x -Wall
CXX = g++
CC = gcc
CPPLIBS = `root-config --cflags --libs`
INCLUDES = include
SRC = src
ODIR = objects

_OBJS = $(PROGRAM).o $(patsubst $(SRC)/%.cc, %.o, $(wildcard $(SRC)/*.cc))
OBJS = $(patsubst %, $(ODIR)/%, $(_OBJS))
ALL = $(OPTIONS) -I$(INCLUDES) $(CPPLIBS)

all: standard

clean:
	rm -rf $(ODIR)/*.o $(PROGRAM)

standard: $(OBJS)
	$(CXX) $(ALL) $(OBJS) -o $(PROGRAM)

$(ODIR)/%.o: $(SRC)/%.cc
	$(CXX) $(ALL) -c $(SRC)/$*.cc -o $(ODIR)/$*.o

$(ODIR)/%.o: %.cpp
	$(CXX) $(ALL) -c $*.cpp -o $(ODIR)/$*.o