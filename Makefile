OPTIONS          = -Wall -O3
LINKER_OPT       = -L/usr/lib -L/usr/local/lib -lstdc++
	

all:
	g++ -Wall cpm.cpp cpmStore.cpp EM.cpp SGD.cpp -o cpm -O3 -lboost_system -lboost_thread -std=c++0x
clean:
	rm -f cpm
