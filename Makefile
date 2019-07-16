CC = g++

CCFLAGS = -Wall -g --std=c++17 -fopenmp -D_GLIBCXX_PARALLEL -O3 -I 3rdparty/Eigen
LDFLAGS = -lm

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
	$(CC) $(CCFLAGS) $^ -o $@

-include $(DEPS)

%.o: %.cpp
	$(CC) $(CCFLAGS) -MMD -c $< -o $@
