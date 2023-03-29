CXX=g++
CXXFLAGS=-std=c++11 -Wall -Wextra
LDFLAGS=-lcurl
TARGET=gptshell

SRCS=gptshell.cpp
OBJS=$(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
