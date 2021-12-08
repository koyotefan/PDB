include $(PCF_DEV_HOME)/makehead
GTEST_DIR=/usr/local/googletest/googletest

SRC_PDB_DIR = $(PCF_DEV_HOME)/SRC/PCF_LIB/PDB

EXE 	= pdbt
EXE_TEST= pdbTest

SRCS 		= $(filter-out main_gtest.cpp, $(wildcard *.cpp))
GTEST_SRCS 	= $(filter-out main.cpp, $(wildcard *.cpp))

HEADERS = $(filter-out, $(wildcard *.hpp))

OBJS 	= $(SRCS:.cpp=.o)
GTEST_OBJS	= $(GTEST_SRCS:.cpp=.o)

CFLAGS += -I$(SRC_PDB_DIR)
CFLAGS += -isystem ${GTEST_DIR}/include
#CFLAGS += -D__T_DEBUG


LDFLAGS += -L$(SRC_PDB_DIR) -lPDB

all :: $(EXE)
$(EXE) : $(OBJS) $(HEADERS)
	$(CXX) -o $(EXE) $(OBJS) $(LDFLAGS)

clean:
	$(RM) $(OBJS) $(EXE) core.* *.d *LOG*

test: $(EXE_TEST)
$(EXE_TEST) : $(GTEST_OBJS) $(HEADERS)
	$(CXX) -o $(EXE_TEST) $(GTEST_OBJS) $(LDFLAGS) -L${GTEST_DIR} -pthread -lgtest
	$(EXE_TEST) nodeId procName ./pdb.json 

