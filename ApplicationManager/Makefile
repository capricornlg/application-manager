
TARGET = appsvc
CXX = g++
CXXFLAGS = -std=c++11 -ggdb3 -Wall -O2 -DBOOST_LOG_DYN_LINK
OEXT = o

# https://blog.csdn.net/humadivinity/article/details/78890754
INCLUDES =
DEP_LIBS = -lpthread -lcrypto -lssl -lACE -lcpprest -lboost_thread -lboost_log -lboost_log_setup -lboost_system -Wl,-Bstatic -ljsoncpp -Wl,-Bdynamic


all : format $(TARGET) 

## source and object files 
SRCS = \
	main.cpp \
	Application.cpp \
	ApplicationShortRun.cpp \
	ApplicationPeriodRun.cpp \
	Configuration.cpp \
	RestHandler.cpp \
	Process.cpp \
	Timer.cpp \
	TimerActionKill.cpp \
	DailyLimitation.cpp \
	TimerAction.cpp \
	../common/Utility.cpp \
	../common/TimeZoneHelper.cpp
		

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
	#dos2unix *.cpp *.h *.json

.PHONY: clean
clean:
	rm -f *.$(OEXT) $(TARGET) 
