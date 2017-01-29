# Blackbird Bitcoin Arbitrage Makefile

override INC_DIR += -I ./src -I/usr/include/mysql/
override LIB_DIR +=
CXXFLAGS := -Wall -pedantic -std=c++11
LDFLAGS  := 
LDLIBS   := -lcrypto -ljansson -lcurl -lmysqlclient

EXEC = blackbird
SOURCES = $(wildcard src/*.cpp) $(wildcard src/*/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

ifndef VERBOSE
  Q := @
else
  Q :=
endif

ifndef DEBUG
  CXXFLAGS += -O2 -DNDEBUG
else
  CXXFLAGS += -O0 -g
endif

all: $(EXEC)

$(EXEC): $(OBJECTS)
	@echo Linking $@:
	$(Q)$(CXX) $(LDFLAGS) $(LIB_DIR) $^ -o $@ $(LDLIBS)

%.o: %.cpp
	@echo Compiling $@:
	$(Q)$(CXX) $(CXXFLAGS) $(INC_DIR) -c $< -o $@

clean:
	$(Q)rm -rf core $(OBJECTS)
