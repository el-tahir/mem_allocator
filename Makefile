# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# Source files
LIB_SRCS = FreeListAllocator.cpp LinearAllocator.cpp
TEST_SRCS = main.cpp
ALL_SRCS = $(LIB_SRCS) $(TEST_SRCS)

# Object files
LIB_OBJS = $(LIB_SRCS:.cpp=.o)
TEST_OBJS = $(TEST_SRCS:.cpp=.o)

# Outputs
LIBRARY = libcustomalloc.a
TEST_EXEC = alloc_test

# --- MAIN TARGETS ---

# Default: Build the test executable
all: $(TEST_EXEC)

# Build the static library only
lib: $(LIB_OBJS)
	ar rcs $(LIBRARY) $(LIB_OBJS)
	@echo "Library $(LIBRARY) created."

# Build and run the executable
test: $(TEST_EXEC)
	./$(TEST_EXEC)

# --- COMPILATION ---

# Link the test executable
$(TEST_EXEC): $(TEST_OBJS) $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# --- SANITIZERS & OPTIMIZATION ---

# Address Sanitizer (Debug -O0)
test_asan: CXXFLAGS += -fsanitize=address -O0
test_asan: clean $(TEST_EXEC)
	@echo "Running with Address Sanitizer (O0)..."
	./$(TEST_EXEC)

# Address Sanitizer (Optimized -O3)
test_asan_opt: CXXFLAGS += -fsanitize=address -O3
test_asan_opt: clean $(TEST_EXEC)
	@echo "Running with Address Sanitizer (O3)..."
	./$(TEST_EXEC)

# Undefined Behavior Sanitizer (Debug -O0)
test_ubsan: CXXFLAGS += -fsanitize=undefined -O0
test_ubsan: clean $(TEST_EXEC)
	@echo "Running with UB Sanitizer (O0)..."
	./$(TEST_EXEC)

# Undefined Behavior Sanitizer (Optimized -O3)
test_ubsan_opt: CXXFLAGS += -fsanitize=undefined -O3
test_ubsan_opt: clean $(TEST_EXEC)
	@echo "Running with UB Sanitizer (O3)..."
	./$(TEST_EXEC)

# --- CLEANUP ---
clean:
	rm -f $(LIB_OBJS) $(TEST_OBJS) $(TEST_EXEC) $(LIBRARY)

.PHONY: all lib test clean test_asan test_asan_opt test_ubsan test_ubsan_opt
