.PHONY: all clean
CPPFLAGS = -g -Wall -std=c++0x -D_GNU_SOURCE
SRCS = general.cpp csocket.cpp csocketfsm.cpp cserveracceptedfsm.cpp cserversocketfsm.cpp cbaseclientfsm.cpp
OBJS = $(SRCS:.cpp=.o)

all: ydd 

clean:
	rm -rf ydd *.o

ydd: ydd.cpp $(OBJS)
	$(CXX) $(CPPFLAGS) -o ydd ydd.cpp $(OBJS)

.cpp.o:
	$(CXX) $(CPPFLAGS) -c $< -o $@
