# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++17

# Source files
SRC = bcdocker.cpp helpresponse.cpp containermanager.cpp

# Header files
HEADERS = helpresponse.h argvparser.h containermanager.h

# Object files (replace .cpp with .o)
OBJ = $(SRC:.cpp=.o)

# Executable name
TARGET = bcdocker

# Default rule
all: $(TARGET)

# Linking rule
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET)

# Compilation rule
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJ) $(TARGET)

# Phony targets
.PHONY: all clean