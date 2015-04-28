#ifndef PUBLISH_SUBSCRIBE_SERVER_H_
#define PUBLISH_SUBSCRIBE_SERVER_H_

#include <boost/asio.hpp>
#include <boost/asio/read.hpp>
#include <boost/thread/thread.hpp>
#include <assert.h>

#include "../host/index.h"

using boost::asio::ip::tcp;
using namespace std;

typedef boost::shared_ptr<tcp::socket> socket_ptr;

class PublishSubscribeServer {
	int localPort;
	int hostCount;
	Index *publishList;
	boost::thread *server;
	void start();
	void session(socket_ptr sock);
public:
	PublishSubscribeServer(int localPort, int hostCount, Index *publishList);
	~PublishSubscribeServer();
};

#endif /* PUBLISH_SUBSCRIBE_SERVER_H_ */