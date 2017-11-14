//
// Created by Trevor  Encinias  on 11/13/17.
//
#include<fstream>
#include <sys/socket.h>
#ifndef P3_SHAREDFUNCTIONS_H
#define P3_SHAREDFUNCTIONS_H
std::ofstream getRecord(std::string );
int sendAll(int ,std::string, std::string);
std::string handleIncomingMessage(int ,std::string);
#endif //P3_SHAREDFUNCTIONS_H
