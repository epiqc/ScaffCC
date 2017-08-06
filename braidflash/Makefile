# the compiler: gcc for C program, define as g++ for C++
# compiles with new gcc version: module unload gcc && module load gcc/4.9
CXX=g++
RM=rm -f

# compiler flags:
CPPFLAGS=-std=c++11 -O3

# define any directories containing header files other than /usr/include
INCLUDES=-I/usr/include

# define library paths in addition to /usr/lib
LFLAGS=-L/usr/lib

# define any libraries to link into executable:
LIBS=

# the build target executable:
TARGET=braidflash

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CXX) $(CPPFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -o $(TARGET) $(TARGET).cpp

clean:
	$(RM) $(TARGET)

