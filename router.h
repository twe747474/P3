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
    std::string fowardingPacket;
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
    void setFowardingPacket(std::string t)
    {
        fowardingPacket = t;
    }
    std::string getFowardingPacket()
    {
        return fowardingPacket;
    }
    void setFwdTable(std::map<int,int> &tmp)
    {
        fwdTable = tmp;
    }
    std::map<int,int>& getFwdTable()
    {
        return fwdTable;
    }
    void addLSP(struct lsp &tmp)
    {
        lspackets.lsps.push_back(tmp);
    }
    struct lspList getLSPlist()
    {
        return lspackets;
    }
    void setLSP(struct lsp &tmp)
    {
        lspacket = tmp;
    }
    struct lsp getLSP()
    {
        return lspacket;
    }
    void addAck(int destRouter, int srcRouter, std::string packet)
    {

        aMap[destRouter].push_back(createAck(srcRouter, packet));
    }
    int getAckSize(int k)
    {
        return aMap.at(k).size();
    }
    ackSet getAck(int k)
    {
      return  aMap.at(k);
    }
    void updateAck(int router, int src)
    {
        bool updated = false;
        for (auto it = aMap[router].begin(); it != aMap[router].end(); ++it)
        {
            if((*it).srcRouter == src){
                (*it).received = true;
                updated = true;
                break;
            }
        }
        if(!updated){
            std::cerr << "Something went wrong! \nrouter: " << router << " was not found in src: " << src << std::endl;
            exit(1);
        }
    }
    struct ack createAck(int srcRouter, std::string packet){
        ack tmp;
        tmp.srcRouter = srcRouter;
        tmp.packet = packet;
        tmp.received = false;
        return tmp;
    }
};
std::string createFowardingPacket(router &r);
//floods network with given packet.
void floodNetwork(std::string , router &, int);
//resend all ack==false
void fowardFlood(router &);
void updateAck(std::string, std::string, router&);
bool waitForAck(int , router &);
bool sendDataGram(int , std::string , router &);
void openAndListen(router &);
void digestMessage( std::string, router & , int);
void createConnection(int , router &);
void createUDP(int , router &);
void Wait(int, router &);
void WaitForNeighbors(int , router &);
void createFwdTable(router &);
void listenMode(router &);
void updateFwdTable(std::map<int,int> &, router &);
void parseAndAdd(std::vector<std::string>, router &);
bool checkTable(std::string , router &);
#endif //P3_ROUTER_H
