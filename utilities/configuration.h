#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <iostream>
#include <stdio.h>
#include <fstream>
#include "../host/host.h"

#define BARRIER					"Barrier:"
#define GRAPH_FILE				"GraphFile:"
#define HOST_COUNT				"HostCount:"
#define THIS_HOST_ADDRESS		"ThisHostAddress:"
#define THIS_HOST_ID			"ThisHostID:"
#define HOSTS					"Hosts:"
#define THREAD_COUNT			"ThreadCount:"
#define VERTEX_COUNT			"VertexCount:"

using namespace std;

class Configuration {
	void readConfigFile(char *cfgFile);
public:
	Configuration(char* configFileName);
	~Configuration();
	void printConfiguration();
	char *barrierAddress;
	int barrierPort;
	int vertexCount;
	int threadCount;
	int machineId;
	char *graphFileName;
	int hostCount;
	Host *hosts;
};

#endif /* CONFIGURATION_H_ */