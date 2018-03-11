COMPILER := gcc
FLAGS	 := -lm -ggdb -Wall -lpthread # -lsocket  
DEFINES  := -DNONE

SRC_FILES := src
SRC_HWAPI := hwAPI
TARGET 	  := bin

# DEFINES := -DDEBUG
#DEFINES := -DBENCHMARK

all: elevator-controler $(TARGET)/hardwareAPI.o

start-elevator-controller-1-5: elevator-controler
	./$(TARGET)/elevator-controler 127.0.0.1 4711 1 5

start-elevator-controller-5-5: elevator-controler
	./$(TARGET)/elevator-controler 127.0.0.1 4711 5 5

elevator-controler: $(SRC_FILES)/main.c $(TARGET)/hardwareAPI.o $(SRC_FILES)/eventQueue.c $(SRC_FILES)/targetQueue.c $(SRC_FILES)/elevatorController.c $(SRC_FILES)/elevatorWorkDistributor.c $(SRC_FILES)/masterEventHandler.c
	@mkdir -p $(TARGET)
	$(COMPILER) $(DEFINES) -o $(TARGET)/$@ $^ $(FLAGS)

start-elevator: elevator/lib/elevator.jar
	java -classpath elevator/lib/elevator.jar elevator.Elevators -top 5 -number 5 -tcp 4711

start-elevator-1: elevator/lib/elevator.jar
	java -classpath elevator/lib/elevator.jar elevator.Elevators -top 5 -number 1 -tcp 4711

start-elevator-test: elevator/lib/elevator.jar
	java -classpath elevator/lib/elevator.jar elevator.Elevators -top 5 -number 5

clean:
	rm bin/*

$(TARGET)/hardwareAPI.o: $(SRC_HWAPI)/hardwareAPI.c
	@mkdir -p bin
	$(COMPILER) $(DEFINES) -o $@ -c $^ 

hwAPI-test: $(TARGET)/hardwareAPI.o $(SRC_HWAPI)/hwAPI-test.c
	@mkdir -p bin
	$(COMPILER) $(DEFINES) -o $(TARGET)/hwAPI-test $(SRC_HWAPI)/hwAPI-test.c $(TARGET)/hardwareAPI.o $(FLAGS)
