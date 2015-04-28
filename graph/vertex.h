#ifndef VERTEX_H_
#define VERTEX_H_

#include <vector>

using namespace std;

class Vertex {
public:
	bool updated;
	int id;
	double value;
	int friendCount;
	int *friendList;
};


#endif /* VERTEX_H_ */