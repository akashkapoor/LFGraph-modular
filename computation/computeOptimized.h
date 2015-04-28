#ifndef COMPUTE_OPTIMIZED_H_
#define COMPUTE_OPTIMIZED_H_

#include "computeAlgorithm.h"

class ComputeOptimized: public ComputeAlgorithm {
public:
	void compute(GraphPartition *graph);
};

#endif /* COMPUTE_OPTIMIZED_H_ */