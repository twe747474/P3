#
#
#This is the Makefile for Project 3 CS457
#
#--------------------------------
CC = g++
CFLAGS  = -pthread -o

default: all
all: manager
 	  
	
manager: manager.cpp 
	$(CC) $(CFLAGS) manager manager.cpp router.cpp sharedFunctions.cpp 

	
clean: 
	rm manager *.out
 
