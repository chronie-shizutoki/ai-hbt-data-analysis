CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
INCLUDES = -Iinclude
SRCS = main.cpp csv_parser.cpp stats.cpp report.cpp i18n.cpp analysis_result.cpp complex_analyzer.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = expense_analyzer

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<

clean:
	rm -f $(OBJS) $(TARGET)
