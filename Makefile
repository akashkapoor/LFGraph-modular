CXXFLAGS =	-std=c++11 -O3 -g -Wall -fmessage-length=0 -I/home/akashkapoor/opt/zookeeper-3.4.6/src/c/include -I/home/akashkapoor/opt/zookeeper-3.4.6/src/c/generated

OBJS =		main.o lfGraph.o computation/computeAlgorithm.o computation/computeOptimized.o communication/communicationManager.o  communication/scatterGatherServer.o communication/publishSubscribeServer.o graph/graphloader.o graph/graphpartition.o utilities/configuration.o graph/vertex.o host/host.o host/index.o communication/barrier.o utilities/timer.o cluster/zookeeper/zookeeper_common.o cluster/zookeeper/server_list.o cluster/zookeeper/key_value.o

LIBS = -DTHREADED -lboost_system -lboost_thread -l zookeeper_mt -lpthread -L/usr/local/lib

TARGET =	main

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
