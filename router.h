//
// Created by Trevor  Encinias  on 11/9/17.
//
#include <sys/socket.h>
#include <vector>
#include <map>
#include "sharedFunctions.h"
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
    bool tcpSent = false;
    lsp lspacket;
    bool goAhead = false;
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



//below are setter and getters of the class router.
public:
    void setGoAhead()
    {
        goAhead = true;
    }

    void tcpTrue()
    {
        tcpSent = true;
    }
    bool getGoAhead()
    {
        return  goAhead;
    }

    bool getTCPStatus()
    {
        return tcpSent;
    }
    void tcpFalse()
    {
        tcpSent = false;
    }
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
    void addLSP(struct lsp tmp)
    {
        lspackets.lsps.push_back(tmp);
    }
    struct lspList getLSPlist()
    {
        return lspackets;
    }

    int getNeighBorsPort(int adddres)
    {
        for(neighbor n : myNeighbors)
        {
            if(n.address == adddres)
            {
                return n.port;
            }

        }
        return -1;
    }
    void addAck(int destRouter, int srcRouter, std::string packet)
    {
        std::ofstream myFile = getRecord(getName());
        //myFile<<currentDateTime()<< " adding ack for:: "<<destRouter << " src:: "<< srcRouter << " packet:;  " << packet<<std::endl;
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
    bool checkAcks()
    {
        for(neighbor n : myNeighbors)
        {
            for(ack k :aMap.at(n.address))
            {
             if(k.received == false)
             {
                 return false;
             }
            }
        }
        return true;

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
            std::cerr << "Something went wrong! \nrouter: " << router << " was not found in src: " << src << " in " << home<< std::endl;
            exit(1);
        }
    }
    //creates an ack structure upon sendign a datagram
    struct ack createAck(int srcRouter, std::string packet){
        ack tmp;
        tmp.srcRouter = srcRouter;
        tmp.packet = packet;
        tmp.received = false;
        return tmp;
    }

};
//creates the fwd packet 5%home%neighbor%...Neighbor
std::string createFowardingPacket(router &r);
//creates an ACK packet 4%from%inRegards
std::string createAckPack(int  , router & );
//this runs a quick look to see if a tcp message is waiting from the manager. TCP connection is set with a timeout. Timeout very by machine but CS linux machines is what this timeout is set for.
void checkTCP(router&);
//this function floods neighbors with given a UDP packet, will not send to src or overall original src.
void floodNetwork(std::string , router &, int , int from = -1);
//does a brief Ack check upon sending any udp messages.
void briefAckCheck(router &);
//resend all packets in ack table if ack == false
void fowardFlood(router &);
//This updates the ack table upon receiving a message.
void updateAck(std::string, std::string, router&);
//this function takes a message and a router , and a port number and sends a datagram to the given port number
void sendDataGram(int, std::string, router &);
//this is the controller of the entire router. This digest each and every message and calls the correct functions based on the signal.
//packet format is signal%messages% very large function since we have a total of 10 signals.
void digestMessage( std::string, router & , int);
//create connection opens a tcp connection to the rotuer.
void createConnection(int , router &);
//create udp opends a udp port.
void createUDP(int , router &);
//this function waits for a message from manager.
void Wait(int, router &);
//creates a fwd table after performing minimum spanning tree.
void createFwdTable(router &);
//this is placed in an infinite loop and does exactly what is says it listens on both udp and tcp connections and then sends the buffer to the digestMessage function
void listenMode(router &);
//this a helper function for createFwd table
void updateFwdTable(std::map<int,int> &, router &);
//this parses incomung lsp packet from all other routers and adds it to the table.
bool parseAndAdd(std::vector<std::string>, router & , std::string);
//this checks the lsp table to see if we have already got lsp packets from the neighbor if so we do not parse and add rather we just send an ack back.
bool checkTable(std::string , router &);
//removes all duplicates from current lsp table
std::vector<neighbor> removeDuplicates(std::vector<neighbor> routingList);
#endif //P3_ROUTER_H
