#include "index.h"

/**
 * Constructor for Index class object.
 */
Index::Index() {
	batch_size = 10000;
	current_size = batch_size;
	data = (int *) malloc(current_size * sizeof(int));
	current_position = 0;
}

/**
 * Destructor for Index class object.
 */
Index::~Index() {
	if(data) {
		free(data);
		data = NULL;
	}
}

/**
 * Adds the given id to the index.
 */
void Index::add(int id) {
	if(current_position >= current_size) {
		current_size += batch_size;
		int *new_data = (int *) realloc(data, sizeof(int) * current_size);
		assert(new_data != NULL);
		data = new_data;
	}
	data[current_position++] = id;
}

/**
 * Clears the index.
 */
void Index::clear() {
	current_position = 0;
}

/**
 * Returns the size of the index.
 */
int Index::getSize() {
	return current_position;
}

/**
 * Return the pointer to data array of the index. 
 */
int* Index::getData() {
	return data;
}