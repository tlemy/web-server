main:
	g++ ./src/main.cpp -std=c++17 -lSDL2 -fext-numeric-literals -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion -Werror -o ./build/main
	./build/main