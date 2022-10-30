
all: 
	g++ -Wall -g -std=c++17 -Iinclude src/P1.cpp -o P1 -pthread
	g++ -Wall -g -std=c++17 -Iinclude src/P2.cpp -o P2 -pthread
	chmod +x P1
	chmod +x P2



