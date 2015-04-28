#ifndef SCATTER_GATHER_SERVER_H_
#define SCATTER_GATHER_SERVER_H_

#include <boost/asio.hpp>
#include <boost/asio/read.hpp>
#include <boost/thread/thread.hpp>
#include <assert.h>

#include "../graph/graphpartition.h"


using boost::asio::ip::tcp;
using namespace std;

typedef boost::shared_ptr<tcp::socket> socket_ptr;

class ScatterGatherServer {
	int localPort;
	GraphPartition *partition;
	boost::thread *server;
	bool optimized = false;
	int gid_to_lid(int id);
	int gid_to_pid(int id);
	void start();
	void session(socket_ptr sock);
public:
	ScatterGatherServer(int localPort, GraphPartition *partition);
	~ScatterGatherServer();
};

#endif /* SCATTER_GATHER_SERVER_H_ */