#include "communicationManager.h"

CommunicationManager::CommunicationManager(Configuration *cfg, GraphPartition *graph) {
	this->cfg = cfg;
	this->graph = graph;
	barrier = new Barrier(cfg->barrierAddress, cfg->barrierPort);
}

bool CommunicationManager::synchronize(int localActiveVertices) {
	bool didSynchronize = false;
	try {
		didSynchronize = barrier->enter(localActiveVertices);	
	} catch (boost::system::system_error const& e) {
		std::cout << "ERROR: could not connect to the barrier server at " 
			<< cfg->barrierAddress << ":" << cfg->barrierPort << " - " << e.what() << "." << std::endl;
		exit(1);
	}
	return didSynchronize;
}

void CommunicationManager::startPublishSubscribeServer() {
	subscribeServer = new PublishSubscribeServer((cfg->hosts)[cfg->machineId].getPort(), cfg->hostCount, graph->publishList);
}

void CommunicationManager::stopPublishSubscribeServer() {
	delete subscribeServer;
}

void CommunicationManager::startScatterGatherServer() {
	scatterServer = new ScatterGatherServer((cfg->hosts)[cfg->machineId].getPort(), graph);
}

void CommunicationManager::stopScatterGatherServer() {
	delete scatterServer;
}

void CommunicationManager::sendSubscriptionRequests() {
	boost::thread_group tgroup;
	for (int i = 0; i < cfg->hostCount; i++) {
		if (i == cfg->machineId)
			continue;
		tgroup.create_thread(boost::bind(&CommunicationManager::subscribe, this, i));
	}
	tgroup.join_all();
}

void CommunicationManager::subscribe(int host_id) {
	boost::asio::io_service io_service;

	tcp::resolver resolver(io_service);
	char s_port[16];
	sprintf(s_port, "%d", (cfg->hosts)[host_id].getPort() + 1);
	tcp::resolver::query query((cfg->hosts)[host_id].getAddress(), s_port);
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	tcp::resolver::iterator end;

	tcp::socket socket(io_service);
	boost::system::error_code error = boost::asio::error::host_not_found;
	while (error && endpoint_iterator != end) {
		socket.close();
		socket.connect(*endpoint_iterator++, error);
	}
	if (error)
		throw boost::system::system_error(error);
	int array_length = (graph->subscribeList)[host_id].getSize();
	//cout << "Local: " << machine_id << ", Remote: " << host_id << " | Count: " << array_length << endl;
	boost::asio::write(socket, boost::asio::buffer(&array_length, sizeof(int)),
			boost::asio::transfer_all(), error);
	boost::asio::write(socket, boost::asio::buffer(&(cfg->machineId), sizeof(int)),
			boost::asio::transfer_all(), error);
	boost::asio::write(socket,
			boost::asio::buffer((graph->subscribeList)[host_id].data,
					array_length * sizeof(int)), boost::asio::transfer_all(),
			error);
	boost::asio::read(socket, boost::asio::buffer(&array_length, sizeof(int)));
}

void CommunicationManager::scatterGatherClient(int host_id) {

	boost::asio::io_service io_service;

	tcp::resolver resolver(io_service);
	char s_port[16];
	sprintf(s_port, "%d", (cfg->hosts)[host_id].getPort());
	tcp::resolver::query query((cfg->hosts)[host_id].getAddress(), s_port);
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	tcp::resolver::iterator end;

	tcp::socket socket(io_service);
	boost::system::error_code error = boost::asio::error::host_not_found;
	while (error && endpoint_iterator != end) {
		socket.close();
		socket.connect(*endpoint_iterator++, error);
	}
	if (error)
		throw boost::system::system_error(error);

	if (!optimized) {
		int array_length = (graph->subscribeList)[host_id].getSize();
		boost::asio::write(socket,
				boost::asio::buffer(&(cfg->machineId), sizeof(int)),
				boost::asio::transfer_all(), error);

		boost::array<double, 32768> buf;
		int l = 0;
		int batch_size =
				(int) buf.size() < array_length ? buf.size() : array_length;
		int remaining_size = array_length;
		while (true) {
			boost::system::error_code error;
			boost::asio::read(socket,
					boost::asio::buffer(buf, sizeof(double) * batch_size));
			for (int i = 0; i < batch_size; i++) {
				int key = (graph->subscribeList)[host_id].data[l];
				(graph->shadowedReadValue)[key] = buf[i];
				l++;
			}
			remaining_size -= batch_size;
			if (remaining_size == 0) {
				break;
			}
			if (remaining_size < batch_size) {
				batch_size = remaining_size;
			}
			if (error == boost::asio::error::eof) {
				break; // Connection closed cleanly by peer.
			} else if (error) {
				throw boost::system::system_error(error); // Some other error.
			}
		}

	} else {
		int array_length;
		boost::asio::write(socket,
				boost::asio::buffer(&(cfg->machineId), sizeof(int)),
				boost::asio::transfer_all(), error);
		boost::asio::read(socket,
				boost::asio::buffer(&array_length, sizeof(int)));
		int *index = new int[array_length];
		boost::asio::read(socket,
				boost::asio::buffer(index, sizeof(int) * array_length));

		boost::array<double, 32768> buf;
		int l = 0;
		int batch_size =
				(int) buf.size() < array_length ? buf.size() : array_length;
		int remaining_size = array_length;
		while (true) {
			boost::system::error_code error;
			boost::asio::read(socket,
					boost::asio::buffer(buf, sizeof(double) * batch_size));
			for (int i = 0; i < batch_size; i++) {
				int key = index[l];
				(graph->shadowedReadValue)[key] = buf[i];
				(graph->vertexUpdated)[key] = true;
				l++;
			}
			remaining_size -= batch_size;
			if (remaining_size == 0) {
				break;
			}
			if (remaining_size < batch_size) {
				batch_size = remaining_size;
			}
			if (error == boost::asio::error::eof) {
				break; // Connection closed cleanly by peer.
			} else if (error) {
				throw boost::system::system_error(error); // Some other error.

			}
		}
		delete[] index;
	}
}

CommunicationManager::~CommunicationManager() {
	delete subscribeServer;
	delete scatterServer;
}