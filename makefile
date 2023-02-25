.PHONY:clean all
SOURCE=$(wildcard *.cpp)
EXE=$(patsubst %.cpp,%,$(SOURCE))

PATH_Com=./common/
PATH_Inc=./include/
COM=readPara
OBJ_com=$(patsubst %,%.o,$(COM))

CC=g++
CFLAG=-Wall -c -std=c++2a

all:$(EXE)

$(EXE):%:%.o $(OBJ_com)
	$(CC) -o $@  $^ -pthread

%.o:%.cpp
	$(CC) $(CFLAG) -o $@ $^

$(OBJ_com):%.o:$(PATH_Com)%.cpp $(PATH_Inc)%.h
	$(CC) $(CFLAG) -o $@ $< 
	
clean:
	rm -f *.o $(EXE) 

