#include "scatterGatherServer.h"

/**
 * Starts ScatterGatherServer that listens to incoming
 * publications.
 */
ScatterGatherServer::ScatterGatherServer(int localPort, GraphPartition *partition) {
	assert(partition != NULL);
	this->localPort = localPort;
	this->partition = partition;
	server = new boost::thread(boost::bind(&ScatterGatherServer::start, this));
}

void ScatterGatherServer::start() {
	boost::asio::io_service io_service;
	tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), localPort));
	while (true) {
		socket_ptr sock(new tcp::socket(io_service));
		a.accept(*sock);
		boost::thread t(boost::bind(&ScatterGatherServer::session, this, sock));
	}
}

void ScatterGatherServer::session(socket_ptr sock) {
	if (!optimized) {
		try {
			int remote_host_id;
			boost::asio::read(*sock,
					boost::asio::buffer(&remote_host_id, sizeof(int)));
			int array_length = (partition->publishList)[remote_host_id].getSize();
			double *data = new double[array_length];

			for (int i = 0; i < (partition->publishList)[remote_host_id].getSize(); i++) {
				int gid = (partition->publishList)[remote_host_id].data[i];
				int pid = gid_to_pid(gid);
				int lid = gid_to_lid(gid);
				data[i] = (partition->vertices)[pid][lid].value;
			}

			boost::asio::write(*sock,
					boost::asio::buffer(data, array_length * sizeof(double)));
			sock->close();
			delete[] data;
		} catch (std::exception& e) {
			std::cerr << "Exception in thread: " << e.what() << "\n";
		}
	} else {
		try {
			int remote_host_id;

			boost::asio::read(*sock,
					boost::asio::buffer(&remote_host_id, sizeof(int)));

			int *index = new int[(partition->publishList)[remote_host_id].getSize()];
			int array_length = 0;
			double *data = new double[(partition->publishList)[remote_host_id].getSize()];
			for (int i = 0; i < (partition->publishList)[remote_host_id].getSize(); i++) {
				int gid = (partition->publishList)[remote_host_id].data[i];
				int pid = gid_to_pid(gid);
				int lid = gid_to_lid(gid);
				if ((partition->vertices)[pid][lid].updated) {
					data[array_length] = (partition->vertices)[pid][lid].value;
					index[array_length++] = gid;
				}
			}

			boost::asio::write(*sock,
					boost::asio::buffer(&array_length, sizeof(int)));
			boost::asio::write(*sock,
					boost::asio::buffer(index, sizeof(int) * array_length));
			boost::asio::write(*sock,
					boost::asio::buffer(data, array_length * sizeof(double)));
			sock->close();
			delete[] data;
			delete[] index;
		} catch (std::exception& e) {
			std::cerr << "Exception in thread: " << e.what() << "\n";
		}
	}
}

int ScatterGatherServer::gid_to_lid(int id) {
	return (id / (partition->partitionCount * partition->hostCount));
}

int ScatterGatherServer::gid_to_pid(int id) {
	return (id / partition->hostCount) % partition->partitionCount;
}


ScatterGatherServer::~ScatterGatherServer() {
	server->interrupt();
	delete server;
}