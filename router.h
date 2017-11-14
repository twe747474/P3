//
// Created by Trevor  Encinias  on 11/9/17.
//
#include <sys/socket.h>
#ifndef P3_ROUTER_H
#define P3_ROUTER_H

void * createRouter(void *);
class router{
    std::string name;
    int udpPort;
public:
    void setName(std::string tempName)
    {
        name = tempName;
    }
    void setUDP(int r)
    {
        udpPort = r ;
    }
    int getUDPPort()
    {
        return udpPort;
    }
    std::string getName()
    {
        return name;
    }

};
void createConnection(int , router &);
void createUDP(int , router &);
#endif //P3_ROUTER_H
