#ifndef HOST_H_
#define HOST_H_

#include "index.h"
#include <string.h>
#include <assert.h>


class Host {
    char address[128];
    int port;
    int machine_id;
    
public:
    Host(const char* address, int port, int machine_id);
    Host();
    Host(const Host &host);
    Host& operator=(const Host &host);
    int getMachineId();
    int getPort();
    char *getAddress();
    void set(const char *address, int port, int machine_id);
private:
    void copy(const Host &host);
};

#endif /* HOST_H_ */