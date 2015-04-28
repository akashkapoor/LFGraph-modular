#include "timer.h"

void Timer::subtract() {
	/* Perform the carry for the later subtraction by updating y. */
	if (stop_time.tv_usec < start_time.tv_usec) {
		int nsec = (start_time.tv_usec - stop_time.tv_usec) / 1000000 + 1;
		start_time.tv_usec -= 1000000 * nsec;
		start_time.tv_sec += nsec;
	}
	if (stop_time.tv_usec - start_time.tv_usec > 1000000) {
		int nsec = (stop_time.tv_usec - start_time.tv_usec) / 1000000;
		start_time.tv_usec += 1000000 * nsec;
		start_time.tv_sec -= nsec;
	}

	/* Compute the time remaining to wait.
	 tv_usec is certainlstart_time positive. */
	delay.tv_sec = stop_time.tv_sec - start_time.tv_sec;
	delay.tv_usec = stop_time.tv_usec - start_time.tv_usec;

}

void Timer::start() {
	gettimeofday(&start_time, NULL);
}

void Timer::stop() {
	gettimeofday(&stop_time, NULL);
}

void Timer::print() {
	this->subtract();
	cout << delay.tv_sec + delay.tv_usec/(double)1000000 << " sec" << endl;
}