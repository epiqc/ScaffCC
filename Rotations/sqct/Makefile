.SUFFIXES:
.SUFFIXES: .cpp .o

CXX=g++
#CXXFLAGS=-Wall -fPIC -c -g -ggdb -O0 -std=c++0x
#CXXWARN=-Wall -Wextra -Wunreachable-code
CXXFLAGS=-fPIC -c -g -ggdb -O0 -std=c++0x
CXXWARN=-Wextra -Wunreachable-code
INC=-I/usr/include/boost
LIB=-L/usr/lib/boost_1_48_0

LDFLAGS=-static-libstdc++ -static-libgcc -static
LDLIBS=-Wl,-Bstatic -lgomp -lpthread -lmpfr -lgmpxx -lgmp -lrt
BOOST=-Wl,-Bstatic -lboost_program_options -lboost_timer -lboost_chrono -lboost_system
SO=-Wall -fPIC

vpath %.cpp
vpath %.h

OBJECTS=hprhelpers.o \
		resring.o \
		rint.o \
		matrix2x2.o \
		vector2.o \
		vector3hpr.o \
		output.o \
		numbersgen.o \
		gatelibrary.o \
		optsequencegenerator.o \
		seqlookupcliff.o \
		exactdecomposer.o \
		epsilonnet.o \
		netgenerator.o \
		unitaryapproximator.o \
		gcommdecomposer.o \
		sk.o \
		skdecomposer.o \
		numbers-stat.o \
		theoremverification.o \
		toptimalitytest.o \
		hoptimalitytest.o

#all: sqct lib test
all: rotZ

rotZ: $(OBJECTS) rotZ.o
	$(CXX) $(LDFLAGS) $(INC) $(LIB) $(OBJECTS) rotZ.o -o rotZ $(BOOST) $(LDLIBS)

sqct: $(OBJECTS) main.o
	$(CXX) $(LDFLAGS) $(INC) $(LIB) $(OBJECTS) main.o -o sqct $(BOOST) $(LDLIBS)

test: lib
	$(CXX) $(LDFLAGS) test.o -o test  -L. -lskdecomp $(BOOST) $(LDLIBS)

%.o : %.cpp
	$(CXX) $(CXXFLAGS) $(INC) $(LIB) $< -o $@

lib: libskdecomp.a
libskdecomp.a: $(OBJECTS)
	#$(CXX) -shared -Wl,-soname,libskdecomp.so.1 -o libskdecomp.so.1.0 $(OBJECTS)
	#ln -sf libskdecomp.so.1.0 libskdecomp.so.1 && ln -sf libskdecomp.so.1 libskdecomp.so
	ar -cvq libskdecomp.a $(OBJECTS) $(BOOST) $(LDLIBS)

install: lib
	cp libskdecomp.so* /usr/lib

clean:
	rm -f rotZ sqct test *.bin *.o *.so *.so.* *.a
