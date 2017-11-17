//
// Created by Trevor  Encinias  on 11/13/17.
//
#include<fstream>
#include <sys/socket.h>
#include <vector>
#ifndef P3_SHAREDFUNCTIONS_H
#define P3_SHAREDFUNCTIONS_H
std::ofstream getRecord(std::string );
int sendAll(int ,std::string, std::string);
std::string handleIncomingMessage(int ,std::string);
std::vector<std::string> splitString(std::string & , char );
const std::string currentDateTime();
#endif //P3_SHAREDFUNCTIONS_H
