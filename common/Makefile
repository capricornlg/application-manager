TARGET = common.a
CXX = g++
CXXFLAGS = -std=c++11 -ggdb3 -Wall -O2 -DBOOST_LOG_DYN_LINK
OEXT = o

INCLUDES =

all : format $(TARGET) 

## source and object files 
SRCS = TimeZoneHelper.cpp Utility.cpp

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
	ar cru $@ $(OBJS)

format:
	#dos2unix *.cpp

.PHONY: clean
clean:
	rm -f *.$(OEXT) $(TARGET) 
