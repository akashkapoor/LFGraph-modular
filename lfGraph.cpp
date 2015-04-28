#include "lfGraph.h"

/**
 * Constructor for LFGraph analytics enginer.
 * Requires Configuration object and ComputeAlgorithm.
 */
LFGraph::LFGraph(Configuration *cfg, ComputeAlgorithm *algorithm) {
	
	this->cfg = cfg;
	this->algorithm = algorithm;
	cfg->printConfiguration();
	graph = new GraphPartition(cfg);
	communicator = new CommunicationManager(cfg, graph);
	communicator->startPublishSubscribeServer();
	communicator->startScatterGatherServer();
}

/**
 * Loads graph from the provided GraphLoader using available configuration.
 */
void LFGraph::loadGraph(GraphLoader* loader) {
	if(loader == NULL) {
		loader = new GraphLoader(cfg, graph);
		loader->load();
		delete loader;
		loader = NULL;
	} else {
		loader->load();
	}

	cout << "Done Loading" << endl;
	communicator->synchronize();
	communicator->sendSubscriptionRequests();
	communicator->synchronize();
}

/**
 * Method to start the computation.
 */
void LFGraph::run() {
	assert(graph != NULL);
	timer.start();
	optimizedCompute();// compute();
	timer.stop();
	timer.print();
}

/**
 * Destructor for LFGraph object.
 */
LFGraph::~LFGraph() {
	delete communicator;
}

void LFGraph::optimizedCompute() {
	//cout << "Inside optimized_compute()" << endl;
	boost::threadpool::pool tpool(boost::thread::hardware_concurrency() - 2);
	for (int step = 0;; step++) {
		for (int i = 0; i < cfg->threadCount; i++) {
			tpool.schedule(boost::bind(&LFGraph::optimizedComputationPerPartition, this, i, step));
		}
		tpool.wait();
		int totalActiveVertices = 0;
		for (int i = 0; i < cfg->threadCount; i++) {
			totalActiveVertices += (graph->partitionActiveVertexCount)[i];
		}
		memset(graph->vertexUpdated, false, graph->totalVertexCount);

		for (int i = 0; i < cfg->threadCount; i++) {
			tpool.schedule(boost::bind(&GraphPartition::shadowPartitionValues, graph, i));
		}

		if (!communicator->synchronize(totalActiveVertices))
			break;

		for (int i = cfg->machineId + 1; i < cfg->hostCount + cfg->machineId; i++) {
			//tpool.schedule(boost::bind(&Langar::client, this, i % machine_count));
			tpool.schedule(boost::bind(
				&CommunicationManager::scatterGatherClient, communicator, i % cfg->hostCount));
		}

		tpool.wait();
		communicator->synchronize();
	}
}

void LFGraph::optimizedComputationPerPartition(int wid, int step) {
	int i, j;

	(graph->partitionActiveVertexCount)[wid] = 0;
	if (step > 0) {
		for (i = 0; i < (graph->partitionVertexCount)[wid]; i++) {
			(graph->vertices)[wid][i].updated = false;
			int friend_count = (graph->vertices)[wid][i].friendCount;
			int min = INT_MAX;
			for (j = 0; j < friend_count; j++) {
				int fid = (graph->vertices)[wid][i].friendList[j];
				if (!(graph->vertexUpdated)[fid]) {
					continue;
				}
				if ((graph->shadowedReadValue)[fid] < min) {
					min = (graph->shadowedReadValue)[fid];
				}
			}

			if ((graph->shadowedReadValue)[(graph->vertices)[wid][i].id] - 1 > min) {
				(graph->vertices)[wid][i].updated = true;
				(graph->partitionActiveVertexCount)[wid]++;
				(graph->vertices)[wid][i].value = min + 1;
			}

		}

	} else {
		for (i = 0; i < (graph->partitionVertexCount)[wid]; i++) {
			if ((graph->vertices)[wid][i].id == 0) {
				(graph->partitionActiveVertexCount)[wid]++;
				(graph->vertices)[wid][i].updated = true;
				(graph->vertices)[wid][i].value = 0;
			} else {
				(graph->vertices)[wid][i].updated = false;
				(graph->vertices)[wid][i].value = INT_MAX;
			}
		}
	}
}

void LFGraph::compute() {
	boost::threadpool::pool tpool(boost::thread::hardware_concurrency() - 3);

	for (int step = 0; step < 5; step++) {
		for (int i = 0; i < (cfg->threadCount); i++) {
			tpool.schedule(boost::bind(&LFGraph::computationPerPartition, this, i, step));
		}
		tpool.wait();

		for (int i = 0; i < (cfg->threadCount); i++) {
			tpool.schedule(boost::bind(&GraphPartition::shadowPartitionValues, graph, i));
		}

		communicator->synchronize();
		if (step == 4)
			break;
		for (int i = cfg->machineId + 1; i < cfg->machineId + cfg->hostCount; i++) {
			tpool.schedule(boost::bind(
				&CommunicationManager::scatterGatherClient, communicator, i % cfg->hostCount));
		}
		tpool.wait();
		communicator->synchronize();
	}
}

void LFGraph::computationPerPartition(int wid, int step) {
	int i, j;
	if (step > 0) {

		for (i = 0; i < (graph->partitionVertexCount)[wid]; i++) {
			int friend_count = (graph->vertices)[wid][i].friendCount;
			double sum = 0;
			for (j = 0; j < friend_count; j++) {

				int fid = (graph->vertices)[wid][i].friendList[j];
				sum += (graph->shadowedReadValue)[fid];
			}
			sum = 0.15 + 0.85 * sum;
			(graph->vertices)[wid][i].value = sum / friend_count;
		}
	} else {
		for (i = 0; i < (graph->partitionVertexCount)[wid]; i++) {
			(graph->vertices)[wid][i].value = 1;
		}
	}
}
