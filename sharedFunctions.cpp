//
// Created by Trevor  Encinias  on 11/13/17.
//
#include <cstring> //added for memset
#include "sharedFunctions.h"
using std::endl;
std::ofstream getRecord(std::string file)
{
    std::ofstream myfile;
    myfile.open(file+".out" , std::ios_base::app);
    return myfile;
}

int sendAll(int sd, std::string packet,std::string fileName)
{
    const char* packetTosend = packet.c_str();
    int len = packet.size();
    int total = 0;
    int bytesleft = len;
    int n;
    std::ofstream myFile = getRecord(fileName);
    myFile<<"Size of packet "<<len<<endl;
    myFile<<"my packet " <<packet<<endl;
    while(total < len)
    {
        n = send(sd, packetTosend+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }
    myFile<<"Size sent "<<total<<endl;
    len = total; // return number actually sent here

    return n==-1?-1:0;
}

std::string handleIncomingMessage(int i , std::string fileName)
{
    int valread;
    char buffer[1024];
    memset(buffer,0,1024);
    std::ofstream myFile = getRecord(fileName);
    if((valread = recv(i,buffer,1024,0)) == 0)
    {
        myFile<<"shit wasnt sent"<<endl;
    }
    else
    {
        myFile<<"from:: "<<i<<" "<<valread<<" from handle::: "<<endl;
        myFile<<buffer<<endl;
        return  buffer;
    }
    return "";
}