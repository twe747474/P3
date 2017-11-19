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
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100;
    if(setsockopt(socketfd, SOL_SOCKET,  SO_RCVTIMEO, (char *)&tv, sizeof(tv)) < 0 )
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
        myFile<<currentDateTime()<<"Got a lsp packet our manager "<<endl;
        brokePacket.erase(brokePacket.begin() + 0);
        r.setHome(brokePacket.at(0));
        brokePacket.erase(brokePacket.begin()+0);
        for(std::string s: brokePacket)
        {
            brokeUp = splitString(s,'|');
            if(brokeUp.size() == 1)
            {
                r.setNumRouters(stoi(brokeUp.at(0)));
                break;
            }
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
        myFile<<"Got a go live message from manager "<<endl;
        string message ="3|Signed|"+r.getHome();
        sendAll(sd , message, r.getName());
        floodNetwork(r.getFowardingPacket() , r , stoi(r.getHome()));
        listenMode(r);
    }
     //ack 4%ack%fromWho%inRegards
    else if(brokePacket.at(0) == "4")
    {
        myFile<<"Got an ack from "<<brokePacket.at(1) <<" in regards to "<<brokePacket.at(2)<<endl;
        updateAck(brokePacket.at(1) , brokePacket.at(2), r);
        //fowardFlood(r);

    }

         //packet with some badass info...
        // asignment packet:::alertNumber%fromRouter%firstNeighbor|port|cost%nNeigbor|nport...
    else if(brokePacket.at(0) == "5")
    {
        //src of packet packet...
        brokePacket.erase(brokePacket.begin() + 0);
        int src = stoi(brokePacket.at(0));
        myFile<<"Paring from:::: "<<src<<endl;
        sendDataGram(r.getNeighBorsPort(src) , createAckPack(src , r), r);
        if(parseAndAdd(brokePacket, r))
        {
            std::string newMessage = "6%" + r.getHome() + "%" + message.substr(2, message.size());
            myFile << " Fwd to neighbors " << newMessage << endl;
            floodNetwork(newMessage, r, src );
        }
        else
        {
            myFile<<" Resent ACK to " <<src<<endl;
        }


    }
        //6%thisRouter%
    else if(brokePacket.at(0) == "6")
    {
        vector<string> newBrokePacket(brokePacket.begin()+1, brokePacket.end());
        int neighbor  = stoi(newBrokePacket.at(1));
        int src = stoi(newBrokePacket.at(0));
        sendDataGram(r.getNeighBorsPort(src), createAckPack(neighbor, r), r);
        if(neighbor != stoi(r.getHome()))
        {
            myFile << currentDateTime() << " got a packet from " << src << "forwarding packet to neighbors";
            string newMessage = "6%" + r.getHome()  +"%" + message.substr(4,message.size());
            if (parseAndAdd(newBrokePacket, r)) {
                //if you break shit look here
                floodNetwork(newMessage, r, src ,  stoi(newBrokePacket.at(1)));
            }
        }
        else
        {
         myFile<<"Resent ack to "<<src<<endl;
        }

    }
        //7%Go
    else if(brokePacket.at(0) == "7")
    {
        myFile<<"Got go ahead from manager  "<<endl;
        createFwdTable(r);
        sendAll(r.getTCPsocket(), "5|"+r.getHome(),r.getName());
    }
        //packet intial packer from manager
    else if(brokePacket.at(0) == "8")
    {
        myFile<<currentDateTime()<<" Received packet from manager..."<<endl;
        if(brokePacket.at(2) == r.getHome())
        {   //this alerts manager that packet has been sent.
            myFile<<currentDateTime()<<" Packet made it! Received packet from:: "<<brokePacket.at(1)<<endl;
            sendAll(r.getTCPsocket(),"4|"+r.getHome() ,r.getName()) ;
        }else
        {
            r.clearAckTable();
            myFile<<currentDateTime()<<" Checking fwd table..... "<<endl;
            int nextStep = r.getFwdTable().at(stoi(brokePacket.at(2)));
            myFile<<currentDateTime()<<" fwd to:: "<<nextStep<<endl;
            string fwdMessae = "9%" + r.getHome() + "%" + message.substr(2, message.length()) + "%";
            r.addAck(nextStep , stoi(r.getHome()) , fwdMessae);
            sendDataGram(r.getNeighBorsPort(nextStep), fwdMessae, r);
        }

    }
    //fwd packet from manager 4%ack%fromWho%inRegards
    else if(brokePacket.at(0) == "9")
    {
        myFile<<currentDateTime()<<": Received a packet from:: "<<brokePacket.at(1)<<endl;
        r.clearAckTable();
        string ack = "4%" +r.getHome() + "%" +brokePacket.at(1) + "%------------------------------------------";
        //this sends an ack/.
        sendDataGram(r.getNeighBorsPort(stoi(brokePacket.at(1))) , ack,r);
        if(brokePacket.at(3) == r.getHome())
        {   //this alerts manager that packet has been sent.
            myFile<<currentDateTime()<<" Packet made it! Received packet from:: "<<brokePacket.at(2)<<endl;
            sendAll(r.getTCPsocket(),"4|"+r.getHome() ,r.getName()) ;
        }
        else
        {
            myFile<<currentDateTime()<<" Checking fwd table..... "<<endl;
            int nextStep = r.getFwdTable().at(stoi(brokePacket.at(2)));
            myFile<<currentDateTime()<<" fwd to:: "<<nextStep<<endl;
            string fwdMessae = "9%" +r.getHome() +"%"  +message.substr(4,message.size());
            r.addAck(nextStep , stoi(r.getHome()) , fwdMessae);
            sendDataGram(r.getNeighBorsPort(nextStep) , fwdMessae , r);

        }
    }
    else if(brokePacket.at(0) == "0")
    {
        //killll
        myFile<<"Received kill message from manager "<<endl;
        myFile<<"Cleaning up........";
        close(r.getUdpSocket());
        close(r.getTCPsocket());
        myFile<<"Looks like my time is up bye"<<endl;
        pthread_cancel(pthread_self());
        usleep(100000);
    }


}
string createAckPack(int src , router &r )
{
    return "4%" + r.getHome() + "%" + std::to_string(src) + "%------------------------------------------";
}

void fowardFlood(router &r)
{
    for(neighbor n : r.getNeighbor())
    {
       for(int i = 0 ; i <r.getAckSize(n.address) ; i++)
       {
           if(r.getAck(n.address).at(i).received == false)
           {
               sendDataGram(r.getNeighBorsPort(n.address) ,r.getAck(n.address).at(i).packet , r );
               briefAckCheck(r);
           }

       }
    }
}
void updateAck(string fromWho , string inRegards, router &r)
{
    std::ofstream myFile = getRecord(r.getName());
    myFile<<currentDateTime()<<"Updating ack in regards::  "<<inRegards << " from :: " << fromWho << endl;
    int routerName = stoi(fromWho);
    int srcRouter = stoi(inRegards);
    if(routerName != stoi(r.getHome())){
        r.updateAck(routerName, srcRouter);
    }

}
bool parseAndAdd(vector<string> packet, router &r)
{
    if(!checkTable(packet.at(0) ,r ))
    {
        lsp l;
        neighbor n;
        l.src = stoi(packet.at(0));
        std::vector<string> brokePacket;
        packet.erase(packet.begin() + 0);
        std::ofstream myFile = getRecord(r.getName());
        for (string s:packet) {
            brokePacket.clear();
            brokePacket = splitString(s, '|');
            //address = src
            if(brokePacket.size() >= 3) {
                if (r.getHome() != brokePacket.at(0)) {
                    myFile << " Parsing and adding ::  " << endl << "source:: " << l.src
                           << endl <<
                           "dest:: " << stoi(brokePacket.at(0)) <<
                           endl <<
                           "cost:: " <<
                           stoi(brokePacket.at(2)) << endl;
                    n.address = l.src;
                    //port = dest
                    n.port = stoi(brokePacket.at(0));
                    n.cost = stoi(brokePacket.at(2));
                    l.neighbors.push_back(n);
                }
            }
        }

        r.addLSP(l);
        return true;

    }
    return false;
}
//to::do need to see if we have already have data.
bool checkTable(std::string src , router &r)
{
    std::ofstream myFile = getRecord(r.getName());
    myFile<<currentDateTime()<<" Checking lsp:::! ";
    vector<lsp> tmp = r.getLSPlist().lsps;
    for(lsp l : tmp)
    {
        if(std::to_string(l.src) == src)
        {
            myFile<<currentDateTime()<<" Have  received an lsp from::: " << src <<endl;
            return true;
        }
    }
    if(src == "6"){

    }
    myFile<<currentDateTime()<<" : does not exis exist from  " << src <<endl;
    return false;

}
void sendDataGram(int port, std::string packet, router &r)
{
    std::ofstream myFile = getRecord(r.getName());
    myFile<<currentDateTime()<<":::: Sending datagram to:: " <<port<<" "<<packet<<endl;
    sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    struct hostent *hp = gethostbyname("127.0.0.1");
    memcpy((void *)&servaddr.sin_addr, hp->h_addr_list[0], hp->h_length);
    std::string test = r.getFowardingPacket();
/* send a message to the server */
    if (sendto(r.getUdpSocket(), packet.c_str(), strlen(packet.c_str()), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("sendto failed");
    }

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

    std::ofstream myFile = getRecord(r.getName());
    myFile<<currentDateTime()<<"::: This is out packet that we fwd:: "<<packet<<endl;
    return packet;
}
void briefAckCheck(router &r)
{
    int recvlen;
    struct sockaddr_in remaddr;     /* remote address */
    socklen_t addrlen = sizeof(remaddr);
    std:: ofstream myFile = getRecord(r.getName());
    char buf[1024];
    {
        recvlen = recvfrom(r.getUdpSocket(), buf, 1024, 0, (struct sockaddr *)&remaddr, &addrlen);
        myFile<<"Bits::: " << recvlen << endl ;
        if (recvlen > 0) {
            buf[recvlen] = 0;
            digestMessage(buf , r , r.getUdpSocket());
            memset(buf,0,1024);
        }
    }
}
void listenMode(router &r)
{
    int recvlen;
    struct sockaddr_in remaddr;     /* remote address */
    socklen_t addrlen = sizeof(remaddr);
    char buf[65536];
    std::ofstream myFile = getRecord(r.getName());
    bool ifPrint = false;
    bool tcpSent = false;
    for (;;) {

        if(r.getTCPStatus())
        {
            //listen for tcp as well...
           checkTCP(r);

        }
        recvlen = recvfrom(r.getUdpSocket(), buf, 65536, 0, (struct sockaddr *)&remaddr, &addrlen);
        if (recvlen > 0 ) {
            buf[recvlen] = 0;
            myFile<<"Bits from listen mode :: "<<recvlen<<endl;
            digestMessage(buf , r , r.getUdpSocket());
            usleep(100 * 1000);
        }
        if(r.getTCPStatus())
        {
            //listen for tcp as well...
            checkTCP(r);

        }
        else
        {
            myFile<<"Foward flood from a timeout "<<endl;
            if(!ifPrint){
                myFile<<r.getNumRouters()<<endl;
                for(lsp l : r.getLSPlist().lsps) {
                    myFile << " src :: " << l.src << endl;
                    for(neighbor n : l.neighbors){
                        myFile<< "Address : " << n.port<<
                              " "
                              <<"cost "
                              <<n.cost<<endl;
                    }
                }
                ifPrint = true;
            }

            if(r.getLSPlist().lsps.size() == r.getNumRouters() -1  && !tcpSent)
            {
                myFile<<currentDateTime()<<" Sending ready message "<<endl;
                string message = "2|" +r.getHome();
                sendAll(r.getTCPsocket() , message, r.getName());
                tcpSent = true;
                r.tcpTrue();
            }
            fowardFlood(r);


        }
    }
}
void checkTCP(router &r)
{
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100;
    fd_set temp;
    FD_SET(r.getTCPsocket(), &temp);
    char buffer[1024];
    memset(buffer,0,1024);
    int multiplexed = select(r.getTCPsocket()+1, &temp, NULL, NULL, &tv);
    if (multiplexed == -1) {
        cout << "Select failed " << strerror(errno) << endl;
    }
    if (FD_ISSET(r.getTCPsocket(), &temp))
    {
        recv(r.getTCPsocket(),buffer,1024,0);
       digestMessage(buffer,r,r.getTCPsocket());
        //  usleep(100 * 1000);

        FD_ZERO(&temp);
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
    else if(valread == -1)
    {

    }
    else
    {
        digestMessage(buffer,r,sd);
    }

}

void createFwdTable(router &r) {
    std::vector<struct lsp> lspList = r.getLSPlist().lsps;
    std::vector<neighbor> routingList;
    std::ofstream myFile = getRecord(r.getName());
    //add its own list to the routing list
    neighbor temp;
    for (auto it :  r.getNeighbor())
    {
        temp.address = stoi(r.getHome());
        temp.port = it.address;
        temp.cost = it.cost;
        routingList.push_back(temp);
    }
    //add all other neighbors lsps to list
    for (auto it : lspList){
        for(auto it2 : (it).neighbors){
            routingList.push_back(it2);
        }
    }
    std::vector<neighbor> newRoutingList = removeDuplicates(routingList);
    Graph g(r.getNumRouters());

    for(neighbor it: routingList){
        //    cout << "src: " << (*it).src << " dest: " << (*it).dest << " weight: " << (*it).weight << endl;
        g.addEdge((it).address, (it).port, (it).cost);
    }

    g.shortestPath(stoi(r.getHome()));
    r.setFwdTable(*g.getFwdTable());

    map<int,int> fwdTable;
    fwdTable = r.getFwdTable();
    for(auto it : fwdTable){
            myFile << (it).first << ":" << (it).second << endl; //dest, neighbor
    }
    updateFwdTable(fwdTable, r);
}

    std::vector<neighbor> removeDuplicates(std::vector<neighbor> routingList){
    std::vector<neighbor> newRoutingList;
    bool contains = false;
    //at this point routingList has both 1->2 and 2->1, need to remove one copy of this
    for(neighbor &i : routingList)
    {
        contains = false;
        for(neighbor &n : newRoutingList){
            if(i.address == n.port && i.port == n.address)
            {
                contains = true;
                break;
            }
        }
        if(!contains)
        {
            newRoutingList.push_back(i);
        }
    }
    return newRoutingList;
}

void updateFwdTable(map<int,int> &tmpFwdTable, router &r)
{
    map<int,int> fwdTable;
    int destNode;
    int currNode;
    int tmp;
    int routerName = stoi(r.getHome());
    for(auto it : tmpFwdTable){
        if((it).second != routerName) {
            destNode = (it).first;
            currNode = (it).second;
            while (currNode != routerName) {
                tmp = currNode;
                currNode = tmpFwdTable[currNode];
            }
            fwdTable[destNode] = tmp;
        }
        else{
            fwdTable[(it).first] = (it).first;
        }
    }
    r.setFwdTable(fwdTable);
}

void floodNetwork(std::string packet, router &r, int src , int from )
{
    //receives packet and needs to forward to all neighbors except src
    //r.addAck(stoi(r.getName()), src, packet);
    std::ofstream myFile = getRecord(r.getName());
    vector<neighbor> neighbors;
    neighbors = r.getNeighbor();
    for(auto it : neighbors){
        if((it).address != src && from != (it).address) {

                r.addAck(((it).address), src, packet);
                myFile << currentDateTime() << " flood network: about to send port" << (it).port << " packet: "
                       << packet;
                sendDataGram((it).port, packet, r);
                briefAckCheck(r);

        }
    }
}
