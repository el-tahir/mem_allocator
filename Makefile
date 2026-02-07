# compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# source files
LIB_SRCS = FreeListAllocator.cpp LinearAllocator.cpp
INTEGRATION_TEST_SRCS = main.cpp
UNIT_TEST_SRCS = unit_test.cpp
ALL_SRCS = $(LIB_SRCS) $(INTEGRATION_TEST_SRCS) $(UNIT_TEST_SRCS)

# Object files
LIB_OBJS = $(LIB_SRCS:.cpp=.o)
INTEGRATION_TEST_OBJS = $(INTEGRATION_TEST_SRCS:.cpp=.o)
UNIT_TEST_OBJS = $(UNIT_TEST_SRCS:.cpp=.o)

# Outputs
LIBRARY = libcustomalloc.a
INTEGRATION_TEST_EXEC = alloc_test
UNIT_TEST_EXEC = unit_test

# --- MAIN TARGETS ---

# Default:
all: $(INTEGRATION_TEST_EXEC) $(UNIT_TEST_EXEC)

# Build the static library only
lib: $(LIB_OBJS)
	ar rcs $(LIBRARY) $(LIB_OBJS)
	@echo "Library $(LIBRARY) created."

# Build and run integration tests
test: $(INTEGRATION_TEST_EXEC)
	./$(INTEGRATION_TEST_EXEC)

# Build and run unit tests
unit_test: $(UNIT_TEST_EXEC)
	./$(UNIT_TEST_EXEC)

# Run all tests
test_all: $(INTEGRATION_TEST_EXEC) $(UNIT_TEST_EXEC)
	@echo "Running integration tests..."
	./$(INTEGRATION_TEST_EXEC)
	@echo ""
	@echo "Running unit tests..."
	./$(UNIT_TEST_EXEC)

# --- COMPILATION ---

# Link the integration test executable
$(INTEGRATION_TEST_EXEC): $(INTEGRATION_TEST_OBJS) $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Link the unit test executable
$(UNIT_TEST_EXEC): $(UNIT_TEST_OBJS) $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- SANITIZERS & OPTIMIZATION ---

# Address Sanitizer (Debug -O0)
test_asan: CXXFLAGS += -fsanitize=address -O0
test_asan: clean all
	@echo "Running integration tests with Address Sanitizer (O0)..."
	./$(INTEGRATION_TEST_EXEC)
	@echo "Running unit tests with Address Sanitizer (O0)..."
	./$(UNIT_TEST_EXEC)

# Undefined Behavior Sanitizer (Debug -O0)
test_ubsan: CXXFLAGS += -fsanitize=undefined -O0
test_ubsan: clean all
	@echo "Running integration tests with UB Sanitizer (O0)..."
	./$(INTEGRATION_TEST_EXEC)
	@echo "Running unit tests with UB Sanitizer (O0)..."
	./$(UNIT_TEST_EXEC)

# --- CLEANUP ---
clean:
	rm -f $(LIB_OBJS) $(INTEGRATION_TEST_OBJS) $(UNIT_TEST_OBJS) \
	      $(INTEGRATION_TEST_EXEC) $(UNIT_TEST_EXEC) $(LIBRARY)

.PHONY: all lib test unit_test test_all clean test_asan test_ubsan
