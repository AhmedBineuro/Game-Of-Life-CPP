main: main.o
	g++ main.o -o main -Wpedantic -Wall -O3  -lsfml-graphics -lsfml-window -lsfml-system
main.o: main.cpp
	g++ -c main.cpp
clean:
	rm *.o
	rm main