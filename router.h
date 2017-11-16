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
    int socket;
};
class router{
    std::string name;
    int udpPort;
    std::vector<neighbor> myNeighbors;
    std::string home;
    int udpSocket;
    int tcpSocket;
    int managerTCP;
public:
    void setTCPsocket(int sd)
    {
        tcpSocket = sd;
    }
    void setmanagerTCP(int sd)
    {
        managerTCP = sd;
    }
    void setSocket(int sd)
    {
        udpSocket = sd;
    }
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
    int getManagerConnection()
    {
        return managerTCP;
    }
    int getTCPsocket()
    {
        return tcpSocket;
    }
    int getUdpSocket()
    {
        return udpSocket;
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
void openAndListen(router &);
void meetNeigbors(router &);
void digestMessage( std::string, router & , int);
void createConnection(int , router &);
void createUDP(int , router &);
void Wait(int, router &);
#endif //P3_ROUTER_H
