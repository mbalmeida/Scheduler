#source files
SRC = .cpp src/foo.cpp
OBJ = $(SRC:.cpp=.o)
OUT = Application

# include directories
INCLUDES = -I. 

# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = -O3
CCC = g++
OTHER =
LIBS = -pthread -lsqlite3 -lboost_program_options

LDFLAGS = -g 
.SUFFIXES: .cpp .c 

default: $(OUT)
.cpp.o:	$(CCC) $(CCFLAGS) $(INCLUDES)  -c $< -o $@
.c.o:	$(CCC) $(CCFLAGS) $(INCLUDES) -c $< -o $@
$(OUT): $(OBJ)	$(CCC) -O3 -o $(OUT) $(OBJ) $(LIBS) 

clean:	rm *.o Makefile.bak 
