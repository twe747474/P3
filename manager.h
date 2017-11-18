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
struct routes
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
    vector<routes> allRoutes;
    int numberOfRouters;
    vector<int> routersSockets;
    fd_set readfds;
    vector<routesAndNeighbors> topology;
    int* readyRouter;

public:
    void pushRouter(routes r)
    {
        allRoutes.push_back(r);
    }
    vector<routes> getRouters()
    {
        return allRoutes;
    }
    void setRouter(int count)
    {
        numberOfRouters = count;
        readyRouter = new int[count];

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


};
void createTwoWay(manager &);
void updateNeighborPorts(int,int , manager &);
void digestMessage(std::string , int , manager &);
routesAndNeighbors findNeighbors(int , manager &);
void giveNeighborHood(int ,std::string, manager & , int);
int connectToRouter(int);
void routerGoLive(manager &);

#endif //P3_MANAGER_H
