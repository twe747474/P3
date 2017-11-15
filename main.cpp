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
#include <cstring>  //added for memset, strerror

using namespace std;

vector<string> splitString(string &info , char delim)
{
    vector<string> brokenInfo;
    string temp = "";
    for(int i = 0; i < info.size(); i++)
    {
        if(info[i] == delim )
        {
            brokenInfo.push_back(temp);
            temp = "";
        }
        else
        {
            temp += info[i];
            if(i+1 >= info.size())
            {
                brokenInfo.push_back(temp);
            }
        }

    }
    return brokenInfo;
}
void openFileR(string &fileName , manager &m)
{
    ifstream inFile;
    inFile.open(fileName);
    string fileInfo;
    bool begin = true;
    vector<string> fileBroken;
    routes router;
    int previousNode = 0;
    routesAndNeghbors ran;
    neighborsAndCost nac;
    ran.home = 0;
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
            }
            else if(stoi(fileInfo) == -1)
            {
                m.pushNeighbors(ran);
                break;
            }
            else
            {
                vector<string> brokenInfo = splitString(fileInfo , ' ');
                int firstNode = stoi(brokenInfo.at(0));
                if(ran.home != firstNode)
                {
                    m.pushNeighbors(ran);
                    ran.neighbors.clear();
                    ran.home = firstNode;
                }
                router.firstNode = stoi(brokenInfo.at(0));
                nac.neighbor = router.secondNode = stoi(brokenInfo.at(1));
                nac.cost = router.cost = stoi(brokenInfo.at(2));
                ran.neighbors.push_back(nac);
                m.pushRouter(router);
            }
        }
    }

}
void spawnRouters(int processCount , manager &m)
{
    int portNumber = 8080;
    pthread_t thread_id;
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
            m.pushRouterSockets(connectToRouter(portNumber));
        }

    }
}
int connectToRouter(int port)
{
    ofstream myfile = getRecord("manager");
    myfile << "port " << port << endl;
    struct addrinfo hints, *res;
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
            myfile << "connected!" << endl;
        }
    char incomingBuffer[1024];
    memset(incomingBuffer, 0, 1024);
    int size;
    myfile.close();
    // close(socketfd);
    // close(currentSock);

    return socketfd;
}
    void multiplex(manager &m)
    {
        struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 500000;
        int t = 0;
        int n = m.getSockets().at(m.getSockets().size() - 1) + 1;
        fd_set temp;
        for (int i : m.getSockets())
        {
            FD_SET(i, &temp);
            int multiplexed = select(n, &temp, NULL, NULL, &tv);
            if (multiplexed == -1)
            {
                cout << "Select failed " << strerror(errno) << endl;
            }
            if (FD_ISSET(i, &temp))
            {
                digestMessage(handleIncomingMessage(i, "manager") , t , m);
                sendAll(i, "You are " + to_string(t), "manager");
                usleep(100 * 1000);
                t++;
                FD_ZERO(&temp);
            }
        }
    }
    //1=portNumber//2=Ready//-1failed
    //packet:: 1|message|routerNumber
    void digestMessage(string message , int router , manager &m)
    {
        ofstream myFile = getRecord("manager");
        myFile<<"Digesting message"<<endl;
        vector<string> sepMessage = splitString(message , '|');
        if(sepMessage.at(0) == "1")
        {
            //expecting port
            myFile<<sepMessage.at(1)<<endl;
            m.getTopolgy(router).myPort = stoi(sepMessage.at(1));
        }
        else if(sepMessage.at(0) == "2")
        {
            //to::do
        }
        else if(sepMessage.at(0) == "3")
        {
            //to::do
        }

    }

int main(int argc, char** argv)
{
    string file = argv[1];
    manager m;
    openFileR(file , m );
    spawnRouters(2 , m);
    multiplex(m);
    for(int i = 0; i < m.topologySize();i++)
    {
        routesAndNeghbors w = m.getTopolgy(i);
        cout<<"Home:: "<<w.home<<endl;
        cout<<"MyPort:: "<<w.myPort<<endl;
        for(neighborsAndCost c: w.neighbors)
        {
            cout<<"Neighbor:: "<<c.neighbor<<" "<<"Cost::  " <<c.cost<<endl;
        }

    }
    return 0;
}

