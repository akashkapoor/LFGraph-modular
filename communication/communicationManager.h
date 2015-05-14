#ifndef COMMUNICATION_MANAGER_H_
#define COMMUNICATION_MANAGER_H_

#include <boost/array.hpp>
#include "../utilities/configuration.h"
#include "../host/index.h"
#include "barrier.h"
#include "publishSubscribeServer.h"
#include "scatterGatherServer.h"
#include <boost/array.hpp>

using namespace std;

class CommunicationManager {
	Barrier* barrier;
	PublishSubscribeServer *subscribeServer;
	ScatterGatherServer *scatterServer;
	Configuration *cfg;
	GraphPartition *graph;
	bool optimized = false;
	void subscribe(int host_id);

public:
	CommunicationManager(Configuration *cfg, GraphPartition *graph);
	~CommunicationManager();
	bool synchronize(int localActiveVertices = 1);
	void startPublishSubscribeServer();
	void startScatterGatherServer();
	
	void stopPublishSubscribeServer();
	void stopScatterGatherServer();

	void sendSubscriptionRequests();
	void scatterGatherClient(int host_id);

};

#endif /* COMMUNICATION_MANAGER_H_ */
