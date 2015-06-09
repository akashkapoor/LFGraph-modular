CXXFLAGS =	-std=c++11 -O3 -g -Wall -fmessage-length=0

OBJS =		main.o lfGraph.o computation/computeAlgorithm.o computation/computeOptimized.o communication/communicationManager.o  communication/scatterGatherServer.o communication/publishSubscribeServer.o graph/graphloader.o graph/graphpartition.o utilities/configuration.o graph/vertex.o host/host.o host/index.o communication/barrier.o utilities/timer.o

LIBS = -lboost_system -lboost_thread -lpthread -L/usr/local/lib

TARGET =	main

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
