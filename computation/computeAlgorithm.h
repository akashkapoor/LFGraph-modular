#ifndef COMPUTE_ALGORITHM_H_
#define COMPUTE_ALGORITHM_H_

#include "../graph/graphpartition.h"

class ComputeAlgorithm {
public:
	virtual void initPartition(GraphPartition *graph) = 0;
	virtual void compute(GraphPartition *graph) = 0;
};

#endif /* COMPUTE_ALGORITHM_H_ */