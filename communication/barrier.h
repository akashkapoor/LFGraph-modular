#ifndef BARRIER_H_
#define BARRIER_H_

#include <cstring>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/asio.hpp>
#include <boost/asio/read.hpp>


using boost::asio::ip::tcp;

class Barrier {
	char address[128];
	int port;
public:
	Barrier(const char *, int);
	bool enter(int = 1);
};

#endif /* BARRIER_H_ */
