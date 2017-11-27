#
#
#This is the Makefile for Project 3 CS457
#
#--------------------------------
CC = g++
CFLAGS  = -pthread -std=c++14 -Wall -Wextra -Wpedantic -Werror -o

default: all

all: manager router
 	  
	
manager: manager.cpp 
	$(CC) $(CFLAGS) manager manager.cc
 	  
router: router.cpp
	$(CC) -pthread $(CFLAGS) router router.cc
	
clean: 
	rm manager router *.out
 
