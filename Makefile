COMPILER := gcc
FLAGS	 := -lm -ggdb -Wall -lpthread -lnsl # -lsocket  
DEFINES  := -DNONE

SRC_FILES := src
SRC_HWAPI := hwAPI
TARGET 	  := bin

# DEFINES := -DDEBUG
#DEFINES := -DBENCHMARK

all: elevator-controler

elevator-controler: main.c
	$(COMPILER) $(DEFINES) -o $(TARGET)/$@ $(SRC_FILES)/$^ $(FLAGS)

start-elevator: elevator/lib/elevator.jar
	java -classpath elevator/lib/elevator.jar elevator.Elevators -top 5 -number 5 -tcp 4711

start-elevator-test: elevator/lib/elevator.jar
	java -classpath elevator/lib/elevator.jar elevator.Elevators -top 5 -number 5

$(TARGET)/hardwareAPI.o: $(SRC_HWAPI)/hardwareAPI.c
	$(COMPILER) $(DEFINES) -o $@ -c $^ 

hwAPI: $(TARGET)/hardwareAPI.o $(SRC_HWAPI)/hwAPI.c
	$(COMPILER) $(DEFINES) -o $(TARGET)/hwAPI $(SRC_HWAPI)/hwAPI.c $(TARGET)/hardwareAPI.o $(FLAGS)
