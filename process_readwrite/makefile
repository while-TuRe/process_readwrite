.PHONY:clean all
SOURCE=$(wildcard test*.cpp)
EXE=$(patsubst %.cpp,%,$(SOURCE))

PATH_Com=./common/
PATH_Inc=./include/
COM=readPara
OBJ_com=$(patsubst %,%.o,$(COM))

CC=g++
CFLAG=-Wall -c

all:$(EXE)

$(EXE):%:%.o $(OBJ_com)
	$(CC) -o $@  $^ 

%.o:%.cpp
	$(CC) $(CFLAG) -o $@ $^

$(OBJ_com):%.o:$(PATH_Com)%.cpp $(PATH_Inc)%.h
	$(CC) $(CFLAG) -o $@ $<
	
clean:
	rm -f *.o $(EXE) 

