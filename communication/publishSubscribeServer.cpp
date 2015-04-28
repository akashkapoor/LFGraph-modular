#include "publishSubscribeServer.h"

/**
 * Starts PublishSubscibeServer that listens to incoming
 * subscription requests and forms the publish list.
 */
PublishSubscribeServer::PublishSubscribeServer(int localPort, int hostCount, Index *publishList) {
	assert(publishList != NULL);
	this->localPort = localPort;
	this->hostCount = hostCount;
	this->publishList = publishList;
	server = new boost::thread(boost::bind(&PublishSubscribeServer::start, this));
}

void PublishSubscribeServer::start() {
	boost::asio::io_service io_service;
	tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), localPort + 1));
	int count = 0;
	while (count < hostCount - 1) {
		socket_ptr sock(new tcp::socket(io_service));
		a.accept(*sock);
		boost::thread t(boost::bind(&PublishSubscribeServer::session, this, sock));
		count++;
	}
}

void PublishSubscribeServer::session(socket_ptr sock) {
	try {
		int subscribeListlength;
		int remoteMachineId;
		boost::asio::read(*sock,
				boost::asio::buffer(&subscribeListlength, sizeof(int)));
		boost::asio::read(*sock,
				boost::asio::buffer(&remoteMachineId, sizeof(int)));
		int *index = new int[subscribeListlength];

		boost::asio::read(*sock,
				boost::asio::buffer(index, sizeof(int) * subscribeListlength));

		for (int i = 0; i < subscribeListlength; i++) {
			publishList[remoteMachineId].add(index[i]);
		}
		boost::asio::write(*sock,
				boost::asio::buffer(&subscribeListlength, sizeof(int)));
		//cout << remote_machine_id << "->" << out_host[remote_machine_id].index.getSize() << endl;
		sock->close();
		delete[] index;
	} catch (std::exception& e) {
		std::cerr << "Exception in thread: " << e.what() << "\n";
	}
}

PublishSubscribeServer::~PublishSubscribeServer() {
	server->interrupt();
	delete server;
}