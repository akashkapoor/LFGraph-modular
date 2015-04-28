#include "graphpartition.h"

/**
 * Creates a GraphPartition object that will use the specified configuration.
 */
GraphPartition::GraphPartition(Configuration* cfg) {
	vertices = NULL;
	partitionVertexCount = NULL;
	
	partitionCount = cfg->threadCount;
	totalVertexCount = cfg->vertexCount;
	hostCount = cfg->hostCount;
	localHostId = cfg->machineId;

	subscribeList = new Index[hostCount];
	publishList = new Index[hostCount];

	partitionActiveVertexCount = new int[partitionCount];
	vertexUpdated = new bool[totalVertexCount];
	shadowedReadValue = new double[totalVertexCount];
}

GraphPartition::~GraphPartition() {
	for (int i = 0; i < partitionCount; i++) {
		if (vertices) {
			if (vertices[i]) {
				for (int j = 0; j < partitionVertexCount[i]; j++) {
					delete[] vertices[i][j].friendList;
				}
				free(vertices[i]);
			}
		}
	}
	if (vertices) {
		delete[] vertices;
	}
	if (partitionVertexCount) {
		delete[] partitionVertexCount;
	}
}

void GraphPartition::shadowPartitionValues(int wid) {
	for (int i = 0; i < partitionVertexCount[wid]; i++) {
		vertexUpdated[vertices[wid][i].id] = vertices[wid][i].updated;
		shadowedReadValue[vertices[wid][i].id] = vertices[wid][i].value;
	}
}