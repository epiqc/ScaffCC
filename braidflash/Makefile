# the compiler: gcc for C program, define as g++ for C++
# compiles with new gcc version: module unload gcc && module load gcc/4.9
CXX=clang++
RM=rm -f

# compiler flags:
CPPFLAGS=-std=c++11 -O3 -Wall

# define any directories containing header files other than /usr/include
INCLUDES=-I/usr/include

# define library paths in addition to /usr/lib
LFLAGS=-L/usr/lib

# define any libraries to link into executable:
LIBS=-lboost_program_options

# implementation source files
SOURCES=argparse.cpp braidflash.cpp

# the build target executable:
TARGET=braidflash

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CXX) $(CPPFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -o $(TARGET) $(SOURCES)

clean:
	$(RM) $(TARGET)

