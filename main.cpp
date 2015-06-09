#include "lfGraph.h"
#include "utilities/configuration.h"
#include "computation/computeAlgorithm.h"
#include "computation/computeOptimized.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {

    if(argc < 3) {
        cerr << "usage: " << argv[0] << " <configuration_file> <this_host_id>" << endl;
        exit(1);
    }

    Configuration *configuration = new Configuration(argv[1], atoi(argv[2]));
    ComputeAlgorithm *algo = new ComputeOptimized();

    LFGraph graphEngine(configuration, algo);
    graphEngine.loadGraph();
    graphEngine.run();

    delete configuration;
    return 0;
}