//
// Created by Trevor  Encinias  on 11/13/17.
//
#include "sharedFunctions.h"
using std::endl;
using std::string;
using std::vector;
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
    myFile<< currentDateTime() << " Packet:: " <<packet<<endl;
    while(total < len)
    {
        n = send(sd, packetTosend+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }
    myFile<< currentDateTime() << " Size sent "<<total<<endl;
    len = total; // return number actually sent here

    return n==-1?-1:0;
}

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

std::string handleIncomingMessage(int i , std::string fileName)
{
    int valread;
    char buffer[1024];
    memset(buffer,0,1024);
    std::ofstream myFile = getRecord(fileName);
    if((valread = recv(i,buffer,1024,0)) == 0)
    {
        myFile<<currentDateTime()<< " shit wasnt sent"<<endl;
    }
    else
    {

        return  buffer;
    }
    return "";
}

const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}