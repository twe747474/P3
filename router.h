//
// Created by Trevor  Encinias  on 11/9/17.
//
#include <sys/socket.h>
#include <vector>
#include <map>
#ifndef P3_ROUTER_H
#define P3_ROUTER_H

void * createRouter(void *);
struct neighbor {
    int address;    //src
    int port;       //dest
    int cost;

    int socket;

    neighbor(int s, int d, int w){
        address = s;
        port = d;
        cost = w;
    }
    neighbor() = default;
};
struct lsp {    //link state packet
    int src;
    std::vector<neighbor> neighbors;

};
class router{
    std::string name;
    int udpPort;
    int nRouters;
    std::vector<neighbor> myNeighbors;
    std::string home;

    int udpSocket;
    int tcpSocket;
    int managerTCP;

    lsp lspacket;
    struct lspList
    {
        std::vector<struct lsp> lsps;
    } lspackets;
    std::map<int,int> fwdTable; //<dest,adjacentNode>
    struct ack
    {
        int srcRouter;
        std::string packet;
        bool received;
    };
    typedef std::vector<struct ack> ackSet;
    typedef std::map<int, ackSet> ackMap;
    ackMap aMap;




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
    void setNumRouters(int n)
    {
        nRouters = n;
    }
    int getNumRouters()
    {
        return nRouters;
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
    void setFwdTable(std::map<int,int> tmp)
    {
        fwdTable = tmp;
    }
    std::map<int,int> getFwdTable()
    {
        return fwdTable;
    }
    void addLSP(struct lsp tmp)
    {
        lspackets.lsps.push_back(tmp);
    }
    struct lspList getLSPlist()
    {
        return lspackets;
    }
    void setLSP(struct lsp tmp)
    {
        lspacket = tmp;
    }
    struct lsp getLSP()
    {
        return lspacket;
    }
    void addAck(int router, struct ack tmp)
    {
        aMap[router].push_back(tmp);
    }
    
};
void openAndListen(router &);
void meetNeigbors(router &);
void digestMessage( std::string, router & , int);
void createConnection(int , router &);
void createUDP(int , router &);
void Wait(int, router &);
void WaitForNeighbors(int , router &);
void createFwdTable(router &);
void updateFwdTable(std::map<int,int> &, router &);
#endif //P3_ROUTER_H
