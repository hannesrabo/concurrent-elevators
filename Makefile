COMPILER := gcc
FLAGS	 := -lm -ggdb -Wall -lpthread -lnsl # -lsocket  
DEFINES  := -DNONE

SRC_FILES := src
SRC_HWAPI := hwAPI
TARGET 	  := bin

# DEFINES := -DDEBUG
#DEFINES := -DBENCHMARK

all: elevator-controler $(TARGET)/hardwareAPI.o

elevator-controler: $(SRC_FILES)/main.c
	$(COMPILER) $(DEFINES) -o $(TARGET)/$@ $^ $(FLAGS)

start-elevator: elevator/lib/elevator.jar
	java -classpath elevator/lib/elevator.jar elevator.Elevators -top 5 -number 5 -tcp 4711

start-elevator-test: elevator/lib/elevator.jar
	java -classpath elevator/lib/elevator.jar elevator.Elevators -top 5 -number 5

clean:
	rm bin/*

$(TARGET)/hardwareAPI.o: $(SRC_HWAPI)/hardwareAPI.c
	$(COMPILER) $(DEFINES) -o $@ -c $^ 

hwAPI-test: $(TARGET)/hardwareAPI.o $(SRC_HWAPI)/hwAPI-test.c
	$(COMPILER) $(DEFINES) -o $(TARGET)/hwAPI-test $(SRC_HWAPI)/hwAPI-test.c $(TARGET)/hardwareAPI.o $(FLAGS)
