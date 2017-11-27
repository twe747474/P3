//
// Created by Trevor  Encinias  on 11/9/17.
//
#include <string>
#include <map>
#include <utility>
using std::vector;
#ifndef P3_MANAGER_H
using std::pair;
#define P3_MANAGER_H
struct routers
{
    int firstNode;
    int secondNode;
    int cost;
};
struct neighborsAndCost
{
    int neighbor;
    int cost;
    int portNumber;
};
struct routesAndNeighbors{
    int home;
    int myPort;
    vector<neighborsAndCost> neighbors;
};

class manager
{
    vector<routers> allRoutes;
    int numberOfRouters;
    vector<int> routersSockets;
    fd_set readfds;
    vector<routesAndNeighbors> topology;
    vector<int> readyRouters;
    vector<std::string> packetInstructions;
    int completedDijkStra = 0;
public:
    int getDijkstra()
    {
        return completedDijkStra;
    }
    void addDijkstra()
    {
        completedDijkStra++;
    }

    void pushPackets(std::string test)
    {
        packetInstructions.push_back(test);
    }
    void pushRouter(routers r)
    {
        allRoutes.push_back(r);
    }
    vector<routers> getRouters()
    {
        return allRoutes;
    }
    void setRouter(int count)
    {
        numberOfRouters = count;
    }
    void pushRouterSockets(int socket)
    {
        FD_SET(socket, &readfds);
        routersSockets.push_back(socket);
    }
    void pushNeighbors(routesAndNeighbors r)
    {
        topology.push_back(r);
    }
    void pushReadyRouter(int router)
    {
       readyRouters.push_back(router);
    }
    bool readyRouterExist(int router)
    {
        for(int i : readyRouters)
        {
            if(i == router)
            {
                return true;
            }
        }
        return false;
    }
    int getReadyRouterSize()
    {
        return readyRouters.size();
    }
    int topologySize()
    {
        return topology.size();
    }

    routesAndNeighbors& getTopolgy(int i)
    {
        return topology.at(i);
    };

    vector<int> getSockets()
    {
        return routersSockets;
    }
    fd_set getSet()
    {
        return readfds;
    }
    int getNumberOfRoutes()
    {
        return numberOfRouters;
    }
    std::vector<std::string>& getPackets()
    {
        return packetInstructions;
    }


};
void potentialKill(manager &  );
void sendReadyMessage(manager &);
void multiplex1(manager &);
void createTwoWay(manager &);
void updateNeighborPorts(int,int , manager &);
void digestMessage(std::string , int , manager &);
routesAndNeighbors findNeighbors(int , manager &);
void giveNeighborHood(int ,std::string, manager & , int);
int connectToRouter(int,int);
void routerGoLive(manager &);

#endif //P3_MANAGER_H
