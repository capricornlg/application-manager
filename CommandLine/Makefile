
TARGET = appc
CXX = g++
CXXFLAGS = -std=c++11 -ggdb3 -Wall -O2
OEXT = o

INCLUDES =
DEP_LIBS = -lpthread -lssl -lcrypto -lcpprest -lboost_system -lboost_thread -lboost_program_options -Wl,-Bstatic -llog4cpp -Wl,-Bdynamic

all : format $(TARGET) 

## source and object files 
SRCS = main.cpp \
	ArgumentParser.cpp \
	../common/Utility.cpp

OBJS = $(SRCS:.cpp=.$(OEXT))

# =============
# File suffixes
# =============
.SUFFIXES: .cpp .$(OEXT)

# ====================
#compile all cpp files
# ===================
%.${OEXT}: %.cpp
	${CXX} ${CXXFLAGS} ${INCLUDES} -c $<;

# Executable binary
$(TARGET): $(OBJS) 
	$(CXX) ${CXXFLAGS} -o $@ $^ $(DEP_LIBS)

format:
	#dos2unix *.cpp

.PHONY: clean
clean:
	rm -f *.$(OEXT) $(TARGET) 
