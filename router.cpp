//
// Created by Trevor  Encinias  on 11/9/17.
//

#include <iostream>
#include "sharedFunctions.h"
#include <netdb.h>
#include <csignal>
#include <vector>
#include "router.h"
#include <unistd.h>
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
    myFile<<"UDP created PortNumber: "<<udpPort<<"Socket:: "<<sock<<endl;
    close(sock);
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
        WaitForNeighbors(clientSocketNumber, r);
        close(socketfd);
        close(clientSocketNumber);
        break;
    }
    //myFile<<"exiting"<<endl;
    //exit(0);

}
void digestMessage(std::string message, router &r)
{
    std::vector<string> brokeUp;
    neighbor n;
    std::vector<string> brokePacket = splitString(message,'%');
    std::ofstream myFile = getRecord(r.getName());
    myFile<<"Packet:: "<<message<<endl;
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
    }
    else{
        //to::do
    }

}

void WaitForNeighbors(int sd, router &r)
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
        digestMessage(buffer,r);
        for(neighbor n:r.getNeighbor())
        {
            myFile<<"Neighbor:: "<<n.address<<endl;
            myFile<<"Port:: "<<n.port<<endl;
            myFile<<"Cost:: "<<n.cost<<endl;
            myFile<<"------------------------"<<endl;
        }
    }
}

