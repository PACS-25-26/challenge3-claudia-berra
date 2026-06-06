CXX = mpic++
EXEC = laplace_jacobi

CPPFLAGS = -Iinclude -DOMPI_SKIP_MPICXX
CXXFLAGS = -std=c++20 -O3 -Wall -Wextra -pedantic -fopenmp
LDFLAGS = -fopenmp
LDLIBS =

SOURCES = \
	src/CommandLine.cpp \
	src/JacobiSolver.cpp \
	src/Output.cpp \
	src/Problem.cpp \
	src/main.cpp

OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: all clean distclean

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJECTS)

distclean: clean
	$(RM) $(EXEC)
