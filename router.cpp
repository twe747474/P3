//
// Created by Trevor  Encinias  on 11/9/17.
//

#include <iostream>
#include "sharedFunctions.h"
#include <netdb.h>
#include <csignal>
#include "router.h"
#include <unistd.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "Dikjstra.cpp"


using std::endl;
using std::cout;
using std::string;

void * createRouter(void * portNumber)
{

    router r;
    r.setName(std::to_string((*(int*)portNumber)));
    std::ofstream myFile = getRecord(r.getName());
    myFile<<"routerCreated "<<*(int*)portNumber<<std::endl;
    createUDP(*(int*)portNumber , r);
    createConnection(*(int*)portNumber , r);
    return 0;
}
void createUDP(int portNumber , router &r)
{
    std::ofstream myFile = getRecord(r.getName());
    myFile<<"Creating udp "<<endl;
    int udpPort = portNumber + 100;
    r.setUDP(udpPort);
    struct addrinfo hints, *res;
    int result , b = 0;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    int opt =1;
    memset(&hints, 0, sizeof hints);
    if((getaddrinfo("127.0.0.1",std::to_string(udpPort).c_str(), &hints, &res)) == -1 )
    {
        printf("something went wrong with status()! %s\n", strerror(errno));
    }
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == -1)
    {
        printf("something failed with socket():::  %s\n", strerror(errno));
    }
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if((b = bind(sock, res->ai_addr, res->ai_addrlen)) == -1)
    {
        printf(" something went wrong with bind()! %s\n", strerror(errno));

    }
    r.setSocket(sock);
    myFile<<"UDP created PortNumber: "<<udpPort<<"Socket:: "<<sock<<endl;
    //close(sock);
}

void createConnection(int portNumber , router &r)
{
    std::ofstream myFile = getRecord(r.getName());
    myFile<<"Creating connection tcp connection to manager "<<endl;
    struct addrinfo hints, *res;
    int socketfd, b , clientSocketNumber,valread;
    struct sockaddr_storage;
    struct sockaddr_in address;
    socklen_t addr_size;
    char buffer[128];
    int opt = 1;
    char name[128];
    gethostname(name,sizeof(name));
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if((getaddrinfo("127.0.0.1",std::to_string(portNumber+1).c_str(), &hints, &res)) == -1 )
    {
        printf("something went wrong with status()! %s\n", strerror(errno));
    }
    if((socketfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
    {
        printf(" something went wrong with socket()! %s\n", strerror(errno));
    }
    if(setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if((b = bind(socketfd, res->ai_addr, res->ai_addrlen)) == -1)
    {
        printf(" something went wrong with bind()! %s\n", strerror(errno));

    }
    if(listen(socketfd,3) != 0)
    {
        printf("something went wrong with listen()! %s\n", strerror(errno));

    }
    memset(buffer , 0 , 128);
    while((clientSocketNumber=accept(socketfd, (struct sockaddr *) &address, &addr_size)))
    {
        myFile<<"Connected to manager" <<std::endl;
        //usleep(1000*100);
        myFile<<"Sending request for a router number and port to manager"<<endl;
        string packet = "1|" + std::to_string(r.getUDPPort()) +"|0000";
        sendAll(clientSocketNumber,packet,r.getName());
        r.setTCPsocket(clientSocketNumber);
        Wait(clientSocketNumber, r);
        break;
    }
    //myFile<<"exiting"<<endl;
    //exit(0);

}
void digestMessage(std::string message, router &r , int sd)
{
    std::vector<string> brokeUp;
    neighbor n;
    std::vector<string> brokePacket = splitString(message,'%');
    std::ofstream myFile = getRecord(r.getName());
    myFile<<"Packet:: "<<message<<endl;
    //neighborhood is coming in.
    if(brokePacket.at(0) == "1")
    {
        brokePacket.erase(brokePacket.begin() + 0);
        r.setHome(brokePacket.at(0));
        brokePacket.erase(brokePacket.begin()+0);
        for(std::string s: brokePacket)
        {
            brokeUp = splitString(s,'|');
            n.address = stoi(brokeUp.at(0));
            n.port = stoi(brokeUp.at(1));
            n.cost = stoi(brokeUp.at(2));
            r.addNeighbor(n);
        }
        std::string message = "3|ready|" +r.getHome();
        //allows manager to know they got the data.
        sendAll(sd , message , r.getName());
        for(neighbor n:r.getNeighbor())
        {

            myFile<<"------------------------"<<endl;
            myFile<<"| Neighbor:: "<<n.address<<endl;
            myFile<<"| Port:: "<<n.port<<endl;
            myFile<<"| Cost:: "<<n.cost<<endl;
            myFile<<"------------------------"<<endl;
        }
        //wait for next step from manager.
        Wait(sd,r);
    }
    //manager will tell router to go and listen for its neighbors.
    else if(brokePacket.at(0) == "2")
    {
        string message ="3|Signed|"+r.getHome();
        sendAll(sd , message, r.getName());
        Wait(r.getTCPsocket() , r);

    }
     //check and connect.
    else if(brokePacket.at(0) == "3")
    {
        string message ="3|Signed|"+r.getHome();
        sendAll(sd , message, r.getName());
    }
     //hello message from router....
    else if(brokePacket.at(0) == "4")
    {

    }
     //hello back from neigbor
    else if(brokePacket.at(0) == "5")
    {

    }

}
void meetNeigbors(router &r)
{
    int tempSd;
    struct sockaddr_in address , serv_addr;
    socklen_t addr_size;
    serv_addr.sin_family = AF_INET;
    std::string packet;
    for(neighbor &n :r.getNeighbor())
    {
        serv_addr.sin_port = htons(n.port);
        if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) == -1)
        {
            cout << "\nInvalid address/ Address not supported \n" << endl;

        }
        std::string packet = "3%hello%"+r.getName();
//        if((tempSd = sendto(r.getUdpSocket(),packet.c_str(), strlen(packet), 0,(struct sockaddr *) &address,(int)sizeof(address))) != -1)
//        {
//            perror("sendto failed");
//        }

    }
    Wait(r.getTCPsocket(),r);
}

//interchangable socket descriptor.
void Wait(int sd, router &r)
{
    int valread;
    char buffer[1024];
    memset(buffer,0,1024);
    std::ofstream myFile = getRecord(r.getName());

    if((valread = recv(sd,buffer,1024,0)) == 0)
    {
        myFile<<"shit wasnt sent"<<endl;
    }
    else
    {
        digestMessage(buffer,r,sd);
    }
}

void createFwdTable(router &r) {
    std::vector<struct lsp> lspList = r.getLSPlist().lsps;
    //(Destination, Cost, NextHop) - (int, int, int)

    std::vector<neighbor> routingList;
    Graph g(r.getNumRouters());


    /*
     //add its own list to the routing list - don't think I need for implentation of Dikjstra. Other lsp's have dest of this router
     for (auto it = r.neighbors.begin(); it != r.neighbors.end(); ++it) {
         routingList.push_back(*it);
     }
     */
    for (auto it = lspList.begin(); it != lspList.end(); ++it){
        for(auto it2 = (*it).neighbors.begin(); it2 != (*it).neighbors.end(); ++it2){
            routingList.push_back(*it2);
        }
    }

    for(auto it = routingList.begin(); it != routingList.end(); ++it){
        //    cout << "src: " << (*it).src << " dest: " << (*it).dest << " weight: " << (*it).weight << endl;
        g.addEdge((*it).address, (*it).port, (*it).cost);
    }

    g.shortestPath(stoi(r.getName()));  //FIXME was 2, should be src node aka stoi(r.getName()) ?
    r.setFwdTable(*g.getFwdTable());


    map<int,int> fwdTable = r.getFwdTable();
    for(auto it = fwdTable.begin(); it != fwdTable.end(); ++it){
            cout << (*it).first << ":" << (*it).second << endl; //dest, neighbor
    }
    updateFwdTable(fwdTable, r);

}

void updateFwdTable(map<int,int> &tmpFwdTable, router &r){
    map<int,int> fwdTable;
    int destNode;
    int currNode;
    int tmp;
    int routerName = stoi(r.getName());
    for(auto it = tmpFwdTable.begin(); it != tmpFwdTable.end(); ++it){
        if((*it).second != routerName) {
            destNode = (*it).first;
            currNode = (*it).second;
            while (currNode != routerName) {
                tmp = currNode;
                currNode = tmpFwdTable[currNode];
            }
            fwdTable[destNode] = tmp;
        }
        else{
            fwdTable[(*it).first] = (*it).first;
        }
    }
    r.setFwdTable(fwdTable);
}