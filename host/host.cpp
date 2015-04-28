#include "host.h"

/**
 * Constructor for the Host class object.
 */
Host::Host(const char* address, int port, int machine_id) {
    set(address, port, machine_id);
}

/**
 * Default constructor for the Host class object.
 * Sets the machine id as 0 and the port as 10000.
 */
Host::Host() {
    this->port = 10000;
    machine_id = 0;
}

/**
 * Copy constructor for the Host class object.
 */
Host::Host(const Host &host) {
    copy(host);
}

/**
 * Constructor for the Host class object.
 */
Host& Host::operator=(const Host &host) {
    copy(host);
    return *this;
}

/**
 * Returns the machine id of the Host.
 */
int Host::getMachineId() {
    return this->machine_id;
}

/**
 * Returns the port of the Host.
 */
int Host::getPort() {
    return this->port;
}

/**
 * Returns the address of the Host.
 */
char* Host::getAddress() {
    return this->address;
}

/**
 * Sets the address, port and machine id of the machine according to 
 * specified arguments. 
 */
void Host::set(const char *address, int port, int machine_id) {
    assert(address != NULL);
    assert(machine_id >= 0);
    assert(port >= 0);

    strcpy(this->address, address);
    this->port = port;
    this->machine_id = machine_id;
}

void Host::copy(const Host &host) {
    strcpy(this->address, host.address);
    this->port = host.port;
    this->machine_id = host.machine_id;
}