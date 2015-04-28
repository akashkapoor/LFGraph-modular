#ifndef TIMER_H_
#define TIMER_H_

#include <iostream>
#include <sys/time.h>

using namespace std;

class Timer {
	struct timeval start_time, stop_time, delay;
	void subtract();
public:
	void start();
	void stop();
	void print();

};



#endif /* TIMER_H_ */
