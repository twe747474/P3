//
// Created by Trevor  Encinias  on 11/9/17.
//
#include <sys/socket.h>
#include <vector>
#ifndef P3_ROUTER_H
#define P3_ROUTER_H

void * createRouter(void *);
struct neighbor{
    int address;
    int port;
    int cost;
};
class router{
    std::string name;
    int udpPort;
    std::vector<neighbor> myNeighbors;
    std::string home;

public:
    void addNeighbor(neighbor n)
    {
        myNeighbors.push_back(n);
    }
    void setHome(std::string h)
    {
        home = h;
    }
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
    std::string getHome()
    {
        return home;
    }

    std::vector<neighbor>& getNeighbor()
    {
        return myNeighbors;
    }

    std::string getName()
    {
        return name;
    }

};
void digestMessage( std::string, router &);
void createConnection(int , router &);
void createUDP(int , router &);
void WaitForNeighbors(int , router &);
#endif //P3_ROUTER_H
