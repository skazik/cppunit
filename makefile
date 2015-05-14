CXX = g++
INCLUDES= -I./
CXXFLAGS = -g $(INCLUDES)
SRCM= ./main.cpp
OBJM = $(SRCM:.cpp=.o)
LINKFLAGS= -lcppunit

unitest1: main.o $(OBJM)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJM) $(LINKFLAGS) $(LINKFLAGSLOG4) $(LIBLOG)

# Default compile

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@
clean:
	rm *.o unitest1 *.xml
