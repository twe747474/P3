//
// Created by Chris on 11/16/17.
//

#ifndef P3_GRAPH_H
#define P3_GRAPH_H

#include <iostream>
#include <set>
#include <list>
#include <vector>
#include <map>

using std::pair;
using std::list;
using std::make_pair;
using std::set;
using std::vector;
using std::map;

# define INF 0x3f3f3f3f

// This class represents a directed graph using
// adjacency list representation
class Graph
{
    int V;    // No. of vertices

    // In a weighted graph, we need to store vertex
    // and weight pair for every edge
    std::list< pair<int, int> > *adj;

    //parent array to hold mst
    map<int,int> *fwdTable; //<dest, nextHop>
    int* distanceTable;

public:
    Graph(int V);  // Constructor

    // function to add an edge to graph
    void addEdge(int u, int v, int w);

    // prints shortest path from s
    void shortestPath(int s);
    map<int,int> *getFwdTable()
    {
        return fwdTable;
    };
    int* getDistTable()
    {
        return distanceTable;
    }
};

#endif //P3_GRAPH_H
