CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -g

LIB_SRCS := FreeListAllocator.cpp LinearAllocator.cpp
LIB_OBJS := $(LIB_SRCS:.cpp=.o)

DEMO_SRCS := main.cpp
INTEGRATION_TEST_SRCS := integration_test.cpp
UNIT_TEST_SRCS := unit_test.cpp

DEMO_OBJS := $(DEMO_SRCS:.cpp=.o)
INTEGRATION_TEST_OBJS := $(INTEGRATION_TEST_SRCS:.cpp=.o)
UNIT_TEST_OBJS := $(UNIT_TEST_SRCS:.cpp=.o)

LIBRARY := libcustomalloc.a
DEMO_EXEC := alloc_demo
INTEGRATION_TEST_EXEC := integration_test
UNIT_TEST_EXEC := unit_test

.PHONY: all demo lib test test-unit test-integration test-asan test-ubsan clean

all: demo test

demo: $(DEMO_EXEC)

lib: $(LIBRARY)

$(LIBRARY): $(LIB_OBJS)
	ar rcs $@ $^

test: test-unit test-integration

test-unit: $(UNIT_TEST_EXEC)
	./$(UNIT_TEST_EXEC)

test-integration: $(INTEGRATION_TEST_EXEC)
	./$(INTEGRATION_TEST_EXEC)

$(DEMO_EXEC): $(DEMO_OBJS) $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(INTEGRATION_TEST_EXEC): $(INTEGRATION_TEST_OBJS) $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(UNIT_TEST_EXEC): $(UNIT_TEST_OBJS) $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test-asan: CXXFLAGS += -fsanitize=address -O0
test-asan: clean
	$(MAKE) test

test-ubsan: CXXFLAGS += -fsanitize=undefined -O0
test-ubsan: clean
	$(MAKE) test

clean:
	rm -f $(LIB_OBJS) $(DEMO_OBJS) $(INTEGRATION_TEST_OBJS) $(UNIT_TEST_OBJS)
	rm -f $(LIBRARY) $(DEMO_EXEC) $(INTEGRATION_TEST_EXEC) $(UNIT_TEST_EXEC)
