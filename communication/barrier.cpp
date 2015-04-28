#include "barrier.h"

using namespace std;

/**
 * Constructs a BarrierWrapper Object to contact the 
 * barrier server at the given address and port.
 */
Barrier::Barrier(const char *address, int port) {
	strcpy(this->address, address);
	this->port = port;
}

/**
 * Synchronizes the computation with the barrier server.
 * Should be called with the active_count of the vertices 
 * at the current node.
 * Returns true if total active nodes are more than 0.
 */
bool Barrier::enter(int active_count) {
	boost::asio::io_service io_service;

	tcp::resolver resolver(io_service);
	char s_port[16];
	sprintf(s_port, "%d", port);
	tcp::resolver::query query(this->address, s_port);
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
	int t_active = 0;
	boost::asio::write(socket, boost::asio::buffer(&active_count, sizeof(int)),
				boost::asio::transfer_all(), error);
	boost::asio::read(socket, boost::asio::buffer(&t_active, sizeof(int)));
	if(t_active == 0) {
		return false;
	}
	return true;
}