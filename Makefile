CC = g++

CCFLAGS = -Wall -g --std=c++2a -fopenmp -O3 -I src
LDFLAGS = 

.PHONY: all clean

BIN = search
SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:%.cpp=%.o)

DEPS = $(OBJECTS:%.o=%.d)

all: $(BIN)

clean:
	-rm bin/$(BIN) $(OBJECTS) $(DEPS)

$(BIN) : bin/$(BIN)

bin/$(BIN): $(OBJECTS)
	mkdir -p $(@D)
	$(CC) $(CCFLAGS) $^ -o $@ $(LDFLAGS)

-include $(DEPS)

%.o: %.cpp
	$(CC) $(CCFLAGS) -MMD -c $< -o $@
