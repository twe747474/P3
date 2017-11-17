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
    myFile<<currentDateTime() << " routerCreated "<<*(int*)portNumber<<std::endl;
    createUDP(*(int*)portNumber , r);
    createConnection(*(int*)portNumber , r);
    return 0;
}
void createUDP(int portNumber , router &r)
{
    std::ofstream myFile = getRecord(r.getName());
    myFile<<currentDateTime() << " Creating udp "<<endl;
    int udpPort = portNumber + 100;
    r.setUDP(udpPort);
    struct sockaddr_in myaddr;      /* our address */
    struct sockaddr_in remaddr;     /* remote address */
    socklen_t addrlen = sizeof(remaddr);/* length of addresses */
    int fd;                         /* our socket */
    /* create a UDP socket */
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket\n");

    }
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 100000;
    if(setsockopt(fd, SOL_SOCKET,  SO_RCVTIMEO, (char *)&tv, sizeof(tv)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    /* bind the socket to any valid IP address and a specific port */
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    // myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(udpPort);

    if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)
    {
        perror("bind failed");
    }

    r.setSocket(fd);
    myFile<< currentDateTime() << " UDP created PortNumber: "<<udpPort<<"Socket:: "<<fd<<endl;

    //close(sock);
}

void createConnection(int portNumber , router &r)
{
    std::ofstream myFile = getRecord(r.getName());
    myFile<< currentDateTime() << " Creating connection tcp connection to manager "<<endl;
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
        myFile<< currentDateTime() << " Connected to manager" <<std::endl;
        //usleep(1000*100);
        myFile<< currentDateTime() << " Sending request for a router number and port to manager"<<endl;
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
    myFile<< currentDateTime() << " Packet:: "<<message<<endl;
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

            myFile<<currentDateTime() << " ------------------------"<<endl;
            myFile<<currentDateTime() << " | Neighbor:: "<<n.address<<endl;
            myFile<<currentDateTime() << " | Port:: "<<n.port<<endl;
            myFile<<currentDateTime() << " | Cost:: "<<n.cost<<endl;
            myFile<<currentDateTime() << " ------------------------"<<endl;
        }
        //wait for next step from manager.
        r.setFowardingPacket(createFowardingPacket(r));
        Wait(sd,r);
    }
    //go live message from router.
    else if(brokePacket.at(0) == "2")
    {
        string message ="3|Signed|"+r.getHome();
        sendAll(sd , message, r.getName());
        floodNetwork(r.getFowardingPacket() , r);
        listenMode(r);
    }
     //ack 4%ack%fromWho%inRegards
    else if(brokePacket.at(0) == "4")
    {
        updateAck(brokePacket.at(1) , brokePacket.at(2), r);
        fowardFlood(r);

    }

         //packet with some badass info...
        // asignment packet:::alertNumber%fromRouter%firstNeighbor|port|cost%nNeigbor|nport...
    else if(brokePacket.at(0) == "5")
    {
        //src of packet packet...
        brokePacket.erase(brokePacket.begin() + 0);
        int src = stoi(brokePacket.at(0));
        parseAndAdd(brokePacket, r);
        std::string newMessage = "5%"+r.getHome() +"%" + message;
        floodNetwork(message,r);

    }

}
void floodNetwork(std::string packet , router &r)
{

}

void fowardFlood(router &r)
{
    for(neighbor n : r.getNeighbor())
    {
       for(int i = 0 ; i <r.getAckSize(n.address) ; i++)
       {
           if(r.getAck(n.address).at(i).received == false)
           {
               sendDataGram(r.getUDPPort() ,r.getAck(n.address).at(i).packet , r );
           }

       }
    }
}
void updateAck(string fromWho , string inRegards, router &r)
{
    int routerName = stoi(fromWho);
    int srcRouter = stoi(inRegards);
    r.updateAck(routerName, srcRouter);
}
void parseAndAdd(vector<string> packet, router &r)
{
    if(!checkTable(packet.at(0) ,r ))
    {
        lsp l;
        neighbor n;
        l.src = stoi(packet.at(0));
        std::vector<string> brokePacket;
        packet.erase(packet.begin() + 0);
        for (string s:packet) {
            brokePacket = splitString(s, '|');
            //address = src
            n.address = l.src;
            //port = dest
            n.port = stoi(brokePacket.at(0));
            n.cost = stoi(brokePacket.at(1));
            l.neighbors.push_back(n);
        }
        r.addLSP(l);
    }

}
//to::do need to see if we have already have data.
bool checkTable(std::string src , router &r)
{
    vector<lsp> tmp = r.getLSPlist().lsps;
    for(lsp l : tmp)
    {
        if(std::to_string(l.src) == src)
        {
            return true;
        }
    }

    return false;

}
bool sendDataGram(int port , std::string packet , router &r )
{
    sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    struct hostent *hp = gethostbyname("127.0.0.1");
    memcpy((void *)&servaddr.sin_addr, hp->h_addr_list[0], hp->h_length);
    std::string test = r.getFowardingPacket();
/* send a message to the server */
    if (sendto(r.getUdpSocket(), test.c_str(), strlen(test.c_str()), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("sendto failed");
    }

    return waitForAck(port,r);
}
//only needs to be used for tcp connection
bool waitForAck(int port ,router &r)
{
    struct sockaddr_in remaddr;
    socklen_t addrlen = sizeof(remaddr);
    char ackBuffer[128];
    if(recvfrom(r.getUdpSocket(), ackBuffer, 128, 0, (struct sockaddr *)&remaddr, &addrlen) < 0)
    {
        return false;
    }
    else
        return true;

}


string createFowardingPacket(router &r)
{
    string packet = "5%" +r.getHome() +"%";

    for(neighbor nac : r.getNeighbor())
    {
        packet.append(std::to_string(nac.address)+"|");
        packet.append(std::to_string(nac.port)+"|");
        packet.append((std::to_string(nac.cost))+'%');
    }
    packet.append(r.getHome() + "%");
    return packet;
}
void listenMode(router &r)
{
    int recvlen;
    struct sockaddr_in remaddr;     /* remote address */
    socklen_t addrlen = sizeof(remaddr);
    char buf[1024];
    for (;;) {

        recvlen = recvfrom(r.getUdpSocket(), buf, 1024, 0, (struct sockaddr *)&remaddr, &addrlen);
        printf("received %d bytes\n", recvlen);
        if (recvlen > 0) {
            buf[recvlen] = 0;
            digestMessage(buf , r , r.getUdpSocket());
        }
    }
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


    map<int,int> fwdTable;
    fwdTable = r.getFwdTable();
    for(auto it = fwdTable.begin(); it != fwdTable.end(); ++it){
            cout << (*it).first << ":" << (*it).second << endl; //dest, neighbor
    }
    updateFwdTable(fwdTable, r);

}

void updateFwdTable(map<int,int> &tmpFwdTable, router &r)
{
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

void floodNetwork(std::string packet, router &r, int src)
{
    //receives packet and needs to forward to all neighbors except src
    //r.addAck(stoi(r.getName()), src, packet);
    vector<neighbor> neighbors;
    neighbors = r.getNeighbor();
    for(auto it = neighbors.begin(); it != neighbors.end(); ++it){
        if((*it).address != src) {
            r.addAck(((*it).address), src, packet);
            sendDataGram((*it).port, packet, r);
        }
    }
}
