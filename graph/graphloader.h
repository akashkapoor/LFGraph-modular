#ifndef GRAPH_LOADER_H_
#define GRAPH_LOADER_H_

#include <assert.h>
#include <boost/thread/thread.hpp>
#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>

#include "../utilities/boost/threadpool.hpp"
#include "../utilities/configuration.h"
#include "graphpartition.h"


using namespace std;

class GraphLoader {
	GraphPartition *graph;
	char* graphFileName;
	void loadLogNormalHelper(int wid, vector<long long> &degree, bool *flag);
	void loadFromFile();
	void loadLogNormal();
public:
	GraphLoader(Configuration *cfg, GraphPartition *graph);	
	void load();
};

#endif /* GRAPH_LOADER_H_ */