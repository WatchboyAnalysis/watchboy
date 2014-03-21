PARAMS:=`root-config --libs --cflags` --std=c++0x

all: cfneutrons monitor pedchi

cfneutrons:
	g++ $(PARAMS) cfneutrons.cpp -o cfneutrons

monitor:
	g++ $(PARAMS) monitor.cpp -o monitor

pedchi:
	g++ $(PARAMS) pedchi.cpp -o pedchi

clean:
	rm -rf cfneutrons monitor pedchi
