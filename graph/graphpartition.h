#ifndef GRAPH_PARTITION_H_
#define GRAPH_PARTITION_H_

#include "vertex.h"
#include "../utilities/configuration.h"

using namespace std;

class GraphPartition {
public:
	Vertex **vertices; 
	int *partitionVertexCount;
	
	Index *subscribeList;
	Index *publishList;

	int* partitionActiveVertexCount;

	double* shadowedReadValue;

	int partitionCount;
	int totalVertexCount;
	int hostCount;
	int localHostId;

	bool *vertexUpdated;
	
	GraphPartition(Configuration* cfg);
	~GraphPartition();
	void shadowPartitionValues(int wid);
};

#endif /* GRAPH_PARTITION_H_ */