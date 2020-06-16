CC = g++
CFLAGS = -std=c++11 -Wall -Werror -O2
LIBS = -lglut -lGL -lGLU

all : main.cpp SoccerBall
				$(CC) -o ball $(CFLAGS) main.cpp $(LIBS) SoccerBallGL.o

SoccerBall : SoccerBallGL.cpp 
				$(CC) -c SoccerBallGL.cpp $(CFLAGS) -o SoccerBallGL.o

.PHONY: clean

clean :
				-rm ball SoccerBallGL.o
