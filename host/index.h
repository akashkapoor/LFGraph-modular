#ifndef INDEX_H_
#define INDEX_H_

#include <stdio.h>
#include <iostream>
#include <assert.h>

class Index {
    int current_size;
    int batch_size;
    int current_position;
public:
    Index();
    ~Index();
    void add(int id);
    void clear();
    int getSize();
    int* getData();
    int *data;
};

#endif /* INDEX_H_ */