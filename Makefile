.PHONY: all clean
CPPFLAGS = -g -Wall -std=c++98 -D_GNU_SOURCE
SRCS = general.cpp
OBJS = $(SRCS:.cpp=.o)

all: ydd 

clean:
	rm -rf ydd *.o

ydd: ydd.cpp $(OBJS)
	$(CXX) $(CPPFLAGS) -o ydd ydd.cpp

.cpp.o:
	$(CXX) $(CPPFLAGS) -c $< -o $@
