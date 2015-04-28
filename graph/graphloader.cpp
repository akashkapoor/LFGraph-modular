#include "graphloader.h"

/**
 * Creates the GraphLoader object. 
 * Requires the configuration to be used to load the graph.
 */
GraphLoader::GraphLoader(Configuration *cfg, GraphPartition *graph) {
	assert(cfg != NULL);
	this->graphFileName = cfg->graphFileName;
	this->graph = graph;

}

/**
 * Loads the graph.
 * Returns the GraphPartition object
 */
void GraphLoader::load() {
	if(graphFileName != NULL) {
		loadFromFile();
	} else {
		cout << "Loading logNormal" << endl;
		loadLogNormal();
	}
}

/**
 * Loads the graph from file.
 */
void GraphLoader::loadFromFile() {
	ifstream inputFile(graphFileName);
	assert(inputFile.is_open());
	int partitionBatchSize = 10000;

	int *partitionAllocatedSize = new int[graph->partitionCount];

	(graph->vertices) = new Vertex *[graph->partitionCount];
	(graph->partitionVertexCount) = new int[graph->partitionCount];
	
	bool *isSubscribed = new bool[graph->totalVertexCount];
	memset(isSubscribed, false, graph->totalVertexCount * sizeof(bool));

	for (int i = 0; i < graph->partitionCount; i++) {
		(graph->vertices)[i] = (Vertex *) malloc(partitionBatchSize * sizeof(Vertex));
		(graph->partitionVertexCount)[i] = 0;
		partitionAllocatedSize[i] = partitionBatchSize;
	}

	for (int i = 0;; i++) {
		int inputVertexId, inputVertexFriendCount, inputVertexFriendId;

		inputFile >> inputVertexId;
		if (inputFile.eof()) {
			break;
		}

		int pid = (inputVertexId / graph->hostCount) % graph->partitionCount;

		if ((graph->partitionVertexCount)[pid] >= partitionAllocatedSize[pid]) {
			partitionAllocatedSize[pid] += partitionBatchSize;
			Vertex *new_v = (Vertex *) realloc((graph->vertices)[pid],
					sizeof(Vertex) * partitionAllocatedSize[pid]);
			assert(new_v != NULL);
			(graph->vertices)[pid] = new_v;
		}

		assert((graph->partitionVertexCount)[pid] == inputVertexId/(graph->partitionCount * graph->hostCount));
		(graph->vertices)[pid][(graph->partitionVertexCount)[pid]].id = inputVertexId;

		inputFile >> inputVertexFriendCount;
		(graph->vertices)[pid][(graph->partitionVertexCount)[pid]].friendCount = inputVertexFriendCount;

		(graph->vertices)[pid][(graph->partitionVertexCount)[pid]].friendList = new int[inputVertexFriendCount];
		int count = 0; 
		for (int j = 0; j < inputVertexFriendCount; j++) {
			inputFile >> inputVertexFriendId;
			if (inputVertexFriendId == inputVertexId) { 
				count++;
				continue;
			}
			(graph->vertices)[pid][(graph->partitionVertexCount)[pid]].friendList[j - count] = inputVertexFriendId;

			if (inputVertexFriendId % graph->hostCount != graph->localHostId) {
				if (!isSubscribed[inputVertexFriendId]) {
					(graph->subscribeList)[inputVertexFriendId % graph->hostCount].add(inputVertexFriendId);
					isSubscribed[inputVertexFriendId] = true;
				}
			}
		}
		(graph->vertices)[pid][(graph->partitionVertexCount)[pid]].friendCount = inputVertexFriendCount - count;

		(graph->partitionVertexCount)[pid]++;
	}

	for (int i = 0; i < graph->hostCount; i++) {
		if (i == graph->localHostId)
			continue;
		sort((graph->subscribeList)[i].getData(), (graph->subscribeList)[i].getData() + (graph->subscribeList)[i].getSize());
		cout << "Local: " << graph->localHostId << ", Remote: " << i << " | Count: "
				<< (graph->subscribeList)[i].getSize() << endl;

	}
	delete[] isSubscribed;
	delete[] partitionAllocatedSize;
}

/**
 * Loads a synthetic log normal graph.
 */
void GraphLoader::loadLogNormal() {
	typedef boost::normal_distribution<> NormalDistribution;
	typedef boost::uniform_int<long long> LongDistribution;
	typedef boost::mt19937_64 RandomNumberGenerator;
	typedef boost::variate_generator<RandomNumberGenerator&, NormalDistribution> NormalGenerator;

	NormalDistribution normal_distribution;
	RandomNumberGenerator generator;
	NormalGenerator normalGenerator(generator, normal_distribution);
	generator.seed(10);

	vector<long long> degree(graph->totalVertexCount);
	cout << "degree: " << graph->totalVertexCount << endl;

	for (int i = 0; i < graph->totalVertexCount; i++) {
		degree[i] = (i > 0 ? degree[i - 1] : 0)
				+ (int) (exp(4 + 1.3 * normalGenerator()) + 0.5);
	}
	cout << degree[graph->totalVertexCount - 1] << endl;

	(graph->vertices) = new Vertex *[graph->partitionCount];
	(graph->partitionVertexCount) = new int[graph->partitionCount];

	bool *isSubscribed = new bool[graph->totalVertexCount];
	memset(isSubscribed, 0, graph->totalVertexCount * sizeof(bool));

	boost::threadpool::pool tpool(graph->partitionCount);

	cout << "Start Loading Vertices." << endl;

	for (int i = 0; i < graph->partitionCount; i++) {
		tpool.schedule(
				boost::bind(&GraphLoader::loadLogNormalHelper, this, i, degree, isSubscribed));
	}
	tpool.wait();

	for(int i = 0 ; i < graph->totalVertexCount ; i++) {
		if(isSubscribed[i]) {
			(graph->subscribeList)[i % graph->hostCount].add(i);
		}
	}
	delete[] isSubscribed;
}

void GraphLoader::loadLogNormalHelper(int wid, vector<long long> &degree, bool *isSubscribed) {
	typedef boost::uniform_int<long long> LongDistribution;
	typedef boost::mt19937_64 RandomNumberGenerator;
	typedef boost::variate_generator<RandomNumberGenerator&, LongDistribution> LongGenerator;

	RandomNumberGenerator generator;
	LongDistribution long_distribution(0, degree[graph->totalVertexCount - 1] - 1);
	LongGenerator longGenerator(generator, long_distribution);

	generator.seed(graph->partitionCount * graph->localHostId + wid);

	int average_degree = degree[graph->totalVertexCount - 1] / graph->totalVertexCount;

	(graph->partitionVertexCount)[wid] = (graph->totalVertexCount / graph->partitionCount) / graph->hostCount;
	(graph->vertices)[wid] = new Vertex[(graph->partitionVertexCount)[wid]];
	for (int j = 0; j < (graph->partitionVertexCount)[wid]; j++) {
		(graph->vertices)[wid][j].id = j * graph->partitionCount * graph->hostCount + wid * graph->hostCount
				+ graph->localHostId;
		(graph->vertices)[wid][j].friendCount =
				(graph->vertices)[wid][j].id % 10 == 0 ? average_degree + 1 : average_degree;
		(graph->vertices)[wid][j].friendList = new int[(graph->vertices)[wid][j].friendCount];

		for (int k = 0; k < (graph->vertices)[wid][j].friendCount;) {
			long long r = longGenerator();
			int fid = std::upper_bound(degree.begin(), degree.end(), r)
					- degree.begin();
			if (fid == (graph->vertices)[wid][j].id)
				continue;
			(graph->vertices)[wid][j].friendList[k++] = fid;

			if (fid % graph->hostCount != graph->localHostId) {
				isSubscribed[fid] = true;
			}
		}
	}
}
