#ifndef LF_GRAPH_H_
#define LF_GRAPH_H_

#include <iostream>
#include <assert.h>
#include "computation/computeAlgorithm.h"
#include "utilities/configuration.h"
#include "graph/graphloader.h"
#include "graph/graphpartition.h"
#include "communication/communicationManager.h"
#include "utilities/timer.h"

using namespace std;

class LFGraph {
	Timer timer;
	Configuration *cfg;
	ComputeAlgorithm *algorithm;
	GraphPartition *graph;
	CommunicationManager *communicator;
	void optimizedCompute();
	void optimizedComputationPerPartition(int wid, int step);
	void compute();
	void computationPerPartition(int wid, int step);
public:
	LFGraph(Configuration *cfg, ComputeAlgorithm *algorithm);
	~LFGraph();
	void loadGraph(GraphLoader* loader = NULL);
	void run();
};

#endif /* LF_GRAPH_H_ */