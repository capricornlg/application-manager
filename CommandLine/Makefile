VERSION=1.0
PACKAGE_NAME=appmanager
BUILD_DATE=$(shell date "+%Y%m%d%H%M")
BUILD_TAG=${PACKAGE_NAME}-${VERSION}-${BUILD_DATE}

TARGET = appc
CXX = g++
CXXFLAGS = -std=c++11 -ggdb3 -Wall -O2
OEXT = o

INCLUDES = -I/usr/local/include
DEP_LIBS = -L/usr/local/lib -L/usr/local/lib64 -lpthread -lssl -lcrypto -lcpprest -lboost_system -lboost_thread -lboost_program_options -Wl,-Bstatic -llog4cpp -Wl,-Bdynamic

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
	${CXX} ${CXXFLAGS} ${INCLUDES} -DBUILD_TAG=${BUILD_TAG} -c $<;

# Executable binary
$(TARGET): $(OBJS) 
	$(CXX) ${CXXFLAGS} -o $@ $^ $(DEP_LIBS)

format:
	#dos2unix *.cpp

.PHONY: clean
clean:
	rm -f *.$(OEXT) $(TARGET) 
