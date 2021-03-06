#include <iostream>
#include <vector>
#include <unistd.h>
#include <fstream>
#include <sys/socket.h>
#include <netdb.h>
#include "router.h"
#include <pthread.h>
#include "manager.h"
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include "sharedFunctions.h"
using namespace std;
void openFileR(string &fileName , manager &m)
{
    ifstream inFile;
    inFile.open(fileName);
    string fileInfo;
    bool begin = true;
    vector<string> fileBroken;
    routers router;
    int previousNode = 0;
    routesAndNeighbors ran;
    neighborsAndCost nac;
    ran.home = 0;
    bool packetIn = false;
    ofstream myFile = getRecord("manager");
    myFile<<currentDateTime()<<" TCP Server started parsing::"<<fileName<<endl;
    if(!inFile)
    {
        cout<<"File failed to open "<<fileName<<endl;
        exit(1);
    }
    else
    {
        while(getline(inFile,fileInfo))
        {
            if(begin)
            {
                begin = false;
                m.setRouter(stoi(fileInfo));
                myFile<<currentDateTime()<<" Number of routers:: "<<fileInfo<<endl;
            }
            else if (packetIn)
            {
               if(fileInfo == "-1")
               {
                   break;
               }
                std::vector<std::string> split = splitString(fileInfo , ' ');
                std::string packet =  "8%" +split.at(0) +"%" +split.at(1);
                m.pushPackets(packet);

            }
            else if(stoi(fileInfo) == -1 && !packetIn)
            {
                m.pushNeighbors(ran);
                if(m.topologySize() != m.getNumberOfRoutes())
                {
                    ran.home+=1;
                    ran.neighbors.clear();
                    routesAndNeighbors test =  findNeighbors(ran.home, m);
                    m.pushNeighbors(test);
                }
                packetIn = true;

            }
            else if(!packetIn)
            {
                vector<string> brokenInfo = splitString(fileInfo , ' ');
                int firstNode = stoi(brokenInfo.at(0));
                if(ran.home != firstNode)
                {
                    m.pushNeighbors(ran);
                    if(firstNode != ran.home+1)
                    {
                        m.pushNeighbors(findNeighbors(ran.home +1 , m));
                    }
                    ran.neighbors.clear();
                    ran.home = firstNode;
                }
                router.firstNode = stoi(brokenInfo.at(0));
                nac.neighbor = router.secondNode = stoi(brokenInfo.at(1));
                nac.cost = router.cost = stoi(brokenInfo.at(2));
                nac.portNumber = 0;
                ran.neighbors.push_back(nac);
                m.pushRouter(router);
            }
        }

    }
    int counter = 0;
    createTwoWay(m);

}
void spawnRouters(int processCount , manager &m)
{
    int portNumber = 15000;
    pthread_t thread_id;
    cout<<"Spawning routers.........."<<endl;
    ofstream myFile = getRecord("manager");
    myFile<<currentDateTime()<<" Spawning routers"<<endl;
    for(int i = 0; i < processCount; i++)
    {
        ++portNumber;
        if(pthread_create( &thread_id , NULL ,  createRouter ,  (void *) &portNumber) < 0)
        {
            perror("could not create thread");
        }
        else
        {   //wait for child to get connection up and going before attempting to connect.
            usleep(100 * 10000);
            m.pushRouterSockets(connectToRouter(portNumber , i) );
        }

    }
}
int connectToRouter(int port , int router)
{
    ofstream myfile = getRecord("manager");
    myfile << currentDateTime() << " connecting to router:: " << router <<" on port:: " << port << endl;
    struct addrinfo hints, R,*res;
    int socketfd;
    struct sockaddr_in serv_addr;
    char name[128];
    int currentSock;
    gethostname(name, sizeof(name));
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_socktype = SOCK_STREAM;
    if ((getaddrinfo(name, to_string(port + 1).c_str(), &hints, &res)) == -1)
    {
        cout << "something went wrong with status()!" << strerror(errno) << endl;
    }
    if ((socketfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
    {
        cout << "something went wrong with socket()! " << strerror(errno) << endl;

    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port + 1);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) == -1)
    {
        cout << "\nInvalid address/ Address not supported \n" << endl;

    }
    currentSock = connect(socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (currentSock == -1)
    {
        close(socketfd);
        perror("client: connect");
    } else
        {
            myfile << currentDateTime() << " tcp connection is up for routers "<<router << endl;
        }
    char incomingBuffer[1024];
    memset(incomingBuffer, 0, 1024);
    int size;
    myfile.close();
    // close(socketfd);
    // close(currentSock);

    return socketfd;
}

void intialMultiplex(manager &m)
    {
        struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 500000;
        int t = 0;
        int n = m.getSockets().at(m.getSockets().size() - 1) + 1;
        fd_set temp;
        FD_ZERO(&temp);
        for (int i : m.getSockets()) {
            FD_SET(i, &temp);
            int multiplexed = select(n, &temp, NULL, NULL, &tv);
            if (multiplexed == -1) {
                cout << "Select failed " << strerror(errno) << endl;
            }
            if (FD_ISSET(i, &temp))
            {
                digestMessage(handleIncomingMessage(i, "manager"), t, m);
              //  usleep(100 * 1000);
                t++;
                FD_ZERO(&temp);
            }
        }
        int counter = 0;
        for (int k: m.getSockets())
        {
            giveNeighborHood(k, to_string(counter), m, counter);
            counter++;
        }
    }
void multiplex1(manager &m)
{
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 500000;
    int t = 0;
    int n = m.getSockets().at(m.getSockets().size() - 1) + 1;
    fd_set temp;
    FD_ZERO(&temp);
    for (int i : m.getSockets()) {
        FD_SET(i, &temp);
        int multiplexed = select(n, &temp, NULL, NULL, &tv);
        if (multiplexed == -1) {
            cout << "Select failed " << strerror(errno) << endl;
        }
        if (FD_ISSET(i, &temp))
        {
            digestMessage(handleIncomingMessage(i, "manager"), t, m);
            //  usleep(100 * 1000);
            t++;
            FD_ZERO(&temp);

        }
    }

}
    void routerGoLive(manager &m)
    {
        ofstream myFile = getRecord("manager");
        myFile << "\n" <<currentDateTime() << "SENDING ROUTER'S THEIR NEIGHBORS" << endl;
        for(int i = 0 ; i < m.getNumberOfRoutes() ; i++)
        {
            myFile<<currentDateTime()<< " Sending packet to router:: "<<i<<endl;
            sendAll(m.getSockets().at(i) , "2%GoLive" , "manager");
            digestMessage(handleIncomingMessage(m.getSockets().at(i),"manager") , 0 , m);

        }
    }
    //asignment packet:::alertNumber%routeAsign%firstNeighbor|port|cost%nNeigbor|nport...
    void giveNeighborHood(int sd , string assignmnet , manager &m , int router)
    {
        ofstream myFile = getRecord("manager");
        string packet = "1%" + assignmnet  + "%";
        for(neighborsAndCost nac : m.getTopolgy(router).neighbors)
        {
            packet.append(to_string(nac.neighbor)+"|");
            packet.append(to_string(nac.portNumber)+"|");
            packet.append((to_string(nac.cost))+'%');
        }
        myFile<<currentDateTime()<< " Sending packet to router:: "<<router<<endl;
        packet.append(to_string(m.getNumberOfRoutes()));
        sendAll(sd , packet , "manager");
        digestMessage(handleIncomingMessage(sd , "manager"), router , m);
    }
    //1=portNumber//2=Connection-up//-1failed//3=signature.
    //packet:: 1|routerNumber
    void digestMessage(string message , int router , manager &m)
    {
        ofstream myFile = getRecord("manager");
        myFile<<currentDateTime() << " Digesting message packet:: "<<message <<endl;
        vector<string> sepMessage = splitString(message , '|');
        if(sepMessage.at(0) == "1")
        {
            //expecting port
            myFile<<currentDateTime() << " Router " << router << " at UDP Port " << sepMessage.at(1)<<endl;
            m.getTopolgy(router).myPort = stoi(sepMessage.at(1));
            updateNeighborPorts(router,stoi(sepMessage.at(1)),m);

        }
            //ready call from router
        else if(sepMessage.at(0) == "2")
        {

            myFile<<currentDateTime()<<sepMessage.at(1)<<" Received ready message from router "<< sepMessage.at(1)<<endl;
            if(!m.readyRouterExist(stoi(sepMessage.at(1))))
            {
                m.pushReadyRouter(stoi(sepMessage.at(1)));
            }
            if(m.getReadyRouterSize() == m.getNumberOfRoutes())
            {
                usleep(2000000 * m.getNumberOfRoutes());
                myFile<<currentDateTime()<<" All ready messages received sending go message to all routers"<<endl;
                sendReadyMessage(m);
                usleep(1000000 * m.getNumberOfRoutes());

            }


        }
            //signature
        else if(sepMessage.at(0) == "3")
        {
            //package signed.
            myFile<<currentDateTime() << " Router:: "<<sepMessage.at(2)<< " acknowledged it got its data" << endl;
        }
            //completed route packet fwd.
        else if(sepMessage.at(0) == "4")
        {
            myFile<<currentDateTime()<<" Received completion message from:: "<< sepMessage.at(1)<<endl;
            usleep(1000000);
            potentialKill(m);


        }
        else if(sepMessage.at(0) == "5")
        {
            myFile << currentDateTime() << " Router " << sepMessage.at(1) << " completed dijkstra" << endl;
            m.addDijkstra();
            if(m.getDijkstra() == m.getNumberOfRoutes())
            {
                std::string packet;
                packet = m.getPackets().at(0);
                int t = stoi(splitString(packet , '%').at(1));
                m.getPackets().at(0) = "";
                myFile<<currentDateTime()<<" Sending first instruction........... "<<packet<<endl;
                cout<<"Sending first instruction........... "<<packet<<endl;
                sendAll(m.getSockets().at(t) , packet , "manager");
            }

        }

    }
    void potentialKill(manager & m )
    {
        std::ofstream myFile = getRecord("manager");
        bool complete = false ;
        if(m.getPackets().size() > 0)
        {
            std::string packet;
            for(string &tk: m.getPackets())
            {
                if(tk != "")
                {
                    std::vector<string> s = splitString(tk,'%');
                    int t = stoi(s.at(1));
                 //   string instruction = s.at(0) + "%" + s.at(1) +"%" +s.at(2);
                    cout<<"Sending next instruction"<<endl;
                    myFile<<currentDateTime()<< " Sending next instruction to router:: "<<t<<endl;
                    sendAll(m.getSockets().at(t) , tk , "manager");
                    usleep(1000000);
//                    m.getPackets().erase(m.getPackets().begin() + 0);
                    tk = "";
                    complete = true;
                    break;
                }

            }
            if(!complete)
            {
                myFile<<currentDateTime()<<" Sending kill message to all routers "<<endl;
                int routers = 0;
                for(int i : m.getSockets())
                {
                    myFile<<currentDateTime()<<" Kill message sent to router "<<routers<<endl;
                    routers++;
                    sendAll(i , "0%Kill" , "manager");
                }
                usleep (1000000 * m.getNumberOfRoutes());
                for(int i : m.getSockets())
                {
                    close(i);
                }
                myFile<<currentDateTime()<<" 100 Hours later, simulation completed if this ain't an A I don't know what is..."<<endl;
                std::cout<<"==================================================>"<<endl;
                cout<<"Finished"<<endl;
                exit(0);
            }

        }
    }
    void sendReadyMessage(manager &m)
    {   string readyMessage = "7%Go";
        //may need to wait for ack idk.
        ofstream myFile = getRecord("manager");
        int routers = 0;
        for(int i : m.getSockets())
        {

            myFile<<currentDateTime()<<" Sending go message to routers:: "<<routers<<endl;
            routers++;
            sendAll(i,readyMessage,"manager");
        }
    }
    void updateNeighborPorts(int router , int port , manager &m )
    {
        for (neighborsAndCost i : m.getTopolgy(router).neighbors)
       {
           int flag = false;
           int t = i.neighbor;
            for (neighborsAndCost &k : m.getTopolgy(i.neighbor).neighbors)
            {
               if (k.neighbor == router) {
                    k.portNumber = port;
                    flag = true;
                    break;

              }
            }
        }
    }
   routesAndNeighbors findNeighbors(int router , manager &m)
   {
        neighborsAndCost nac1;
        routesAndNeighbors r;
        r.home = router;
     for(int i = 0 ; i < m.topologySize();i++)
     {
         for(neighborsAndCost nac : m.getTopolgy(i).neighbors)
         {
             if(nac.neighbor == r.home)
             {
                 nac1.neighbor = m.getTopolgy(i).home;
                 nac1.cost = nac.cost;
                 r.neighbors.push_back(nac1);
                 break;
             }
         }

     }
        return r;

    }
    void createTwoWay(manager &m)
    {
        for(int i = 0 ; i < m.topologySize() ; i++)
        {
            for(neighborsAndCost k : m.getTopolgy(i).neighbors)
            {
                bool flag = false;
                for(neighborsAndCost tl : m.getTopolgy(k.neighbor).neighbors)
                {
                    if(i == tl.neighbor)
                    {
                        flag = true;
                        break;
                    }

                }
                if(flag != true)
                {
                 neighborsAndCost temp;
                    temp.neighbor = i;
                    temp.portNumber = m.getTopolgy(i).myPort;
                    temp.cost = k.cost;
                    m.getTopolgy(k.neighbor).neighbors.push_back(temp);
                }
            }

        }
    }

int main(int argc, char** argv)
{
    cout<<"Simulating...."<<endl;
    string file = argv[1];
    cout<<"==========>"<<endl;
    manager m;
    openFileR(file , m );
    cout<<"=====================>"<<endl;
    spawnRouters(m.getNumberOfRoutes() , m);
    cout<<"===================================>"<<endl;
    intialMultiplex(m);
    routerGoLive(m);
   while(true)
    {
       multiplex1(m);
    }
}

