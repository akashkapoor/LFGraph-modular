#include "configuration.h"

/**
 * Generates the configuration for LFGraph 
 * according to the configuration file, specified
 * thread count and vertex count.
 */
Configuration::Configuration(char* configFileName, int hostId) {
	barrierAddress = NULL;
	barrierPort = -1;
	machineId = hostId;
	graphFileName = NULL;
	hostCount = -1;
	hosts = NULL;
    vertexCount = -1;
    threadCount = -1;
	readConfigFile(configFileName);
}

/**
 * Prints the configuration. FOR DEBUGGING PURPOSES.
 */
void Configuration::printConfiguration() {
	cout << barrierAddress << endl << barrierPort << endl << vertexCount << endl << threadCount << endl;
	cout << machineId << endl << (graphFileName == NULL ? "NULL" : graphFileName)  << endl << hostCount << endl; 
	cout << "Hosts: " << endl;
	for(int i = 0; i < hostCount; i++) {
		cout << hosts[i].getAddress()<<":"<<hosts[i].getPort()<<"-"<<hosts[i].getMachineId()<<endl;
	}
}

/**
 * Destructs the Configuration object.
 */
Configuration::~Configuration() {
    if(barrierAddress != NULL) {
        delete barrierAddress;
    }
    barrierPort = -1;
    machineId = -1;
    if(graphFileName != NULL) {
        delete graphFileName; 
    }
    if(hosts != NULL) {
        delete[] hosts;
        hosts = NULL;
    }
    hostCount = -1;
    vertexCount = -1;
    threadCount = -1;    
}

void Configuration::readConfigFile(char *cfgFile) {
    ifstream in_stream;
    string line;
    in_stream.open(cfgFile);
    while(!in_stream.eof()) 
    {
        in_stream >> line;
        if(line.compare(BARRIER) == 0) {
            in_stream >> line;
            barrierAddress = new char[128];
            sscanf(line.c_str(), "%[^:]:%d", barrierAddress, &barrierPort);
        } 
        else if (line.compare(GRAPH_FILE) == 0) {
            in_stream >> line;
            if(line.compare("NULL") == 0) {
                graphFileName = NULL;
            } else {
                graphFileName = new char [line.length()+1];
                std::strcpy (graphFileName, line.c_str());
            }
        }
        else if (line.compare(HOST_COUNT) == 0) {
            in_stream >> line;
            sscanf(line.c_str(), "%d", &hostCount);
        }
        else if (line.compare(THREAD_COUNT) == 0) {
            in_stream >> line;
            sscanf(line.c_str(), "%d", &threadCount);
        }
        else if (line.compare(VERTEX_COUNT) == 0) {
            in_stream >> line;
            sscanf(line.c_str(), "%d", &vertexCount);
        }
        else if (line.compare(THIS_HOST_ID) == 0) {
            in_stream >> line;
            sscanf(line.c_str(), "%d", &machineId); 
        }
        else if (line.compare(HOSTS) == 0) {
            hosts = new Host[hostCount];
            int i = 0;
            for(i=0; i<hostCount; i++) {
                in_stream >> line;
                char *address = new char[128];
                int port = -1;
                int id = -1;
                sscanf(line.c_str(), "%[^:]:%d-%d", address, &port, &id);
                hosts[i].set(address, port, id);
            }
        }
    }
    in_stream.close();
}