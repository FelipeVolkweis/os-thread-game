CC = g++
APP_NAME = exec
CFLAGS = -Wall -std=c++20
INCLUDES = -I ./include -pthread
LINKS = -lsfml-graphics -lsfml-window -lsfml-system
SRC = src/*.cpp

all:
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC) -o $(APP_NAME) ${LINKS}

run:
	./$(APP_NAME)

clean:
	rm exec