
This code was written for educational purposes by Trevor Encinias and Chris Schaffer 
-----------------------------------------------------------------------------------------------------------------------
Included in package manager.cpp router.cpp Graph sharedFunctions.cpp makefile sharedFunction.h manager.h router.h Dijkstra.h routerFile.txt 
-----------------------------------------------------------------------------------------------------------------------
Program works as intended.
-----------------------------------------------------------------------------------------------------------------------
Resources:
-blood, sweat and tears. 
-coffee and monster. 
———————————————————————————————————————————————————————————————————————————————————————————-
Makefile

To make the file run make all. This will compile and create one executable. This one excitable is all you need in order to run the file. 
In order to run this program place run make all. This will create an executable called a.out. Run ./a.out file path to your routerFile.txt. Below is an explanation on what the routerFile.txt is. 
We ports 8081 and above. Example if you are simulating 10 routers you will have a tcp connection on ports 150000 and above. Upon running the program you see a console output that reads “Simulating…..” followed by multiple progress bars. A successfully completed program output will look like this:

Simulating....
==========>
=====================>
===================================>
==================================================>
Finished

The number of routers and instructions will determine the wait time for this program to execute. An execution time for a file such as below would be 60-85 seconds we were not trying to win a race. 
Inside of the .h files contains function declaration along with commented descriptions of each function. 
-----------------------------------------------------------------------------------------------------------------------
FILE routerFile.txt

The input file routerFile.txt contains a network topology description. 
To run the program you need a routerFile.txt file

The format is as follows:

•The first line of this table contains a single integer N. This means that there are N
	nodes in the network, whose addresses are 0, 1, 2, ..., N-1.
•This line is followed by several lines, one for each point-to-point link in the
	network. Each line has 3 integers separated by whitespace <X, Y, C>. X and Y
	are node numbers between 0 and N-1, and C is a positive integer that represents
	the cost of a link between X and Y.
•A -1, which indicates the end of the list of links in the input file
•The following lines in the form <X, Y> indicate the source network node X and the destination node Y. 
•A -1, to indicate the end of packet forwarding tests and the end of the file. 

Here is a sample network topology description:

10
0 9 40
0 4 60
0 2 40
1 8 70
2 6 70
3 9 20
3 8 70
3 5 70
4 7 70
4 6 40
5 9 70
5 7 40
7 9 60
8 9 70
-1
0 4
8 6
-1

---------------------------------------------------------------------------------------------------------------------------

Program design: 
The program is designed to be a simulation of Link State Routing protocol. On start of this program the manager parses the file into predefined structures called neighborsandCost. Those contain as titled and int neighbor , int cost and int port number. Once the file is parsed it spawns routers. When a router gets spawned it opens up a UDP port and then opens up a TCP connection to the manger. The router then sends a tcp message to the manager giving the manager its port number the manger then takes that port number and fills in the vector or neighborsAnCost with the port number. Once all routers that were spawned have reported back to the manager then sends each router a packet that contains who they are, an array of the routers neighborsandcost along with the amount of routers on the network. Upon receiving this information the router begin performing limited broadcasting. We implemented a what called an ACK table. This table consisted of structure that was titled ACK. Inside of this ACK is a sources address the packet and a boolean ACKed. Any message that is not an ack message sent to a UDP port gets an ACK entry into the table. Upon a timeout we check this ack table and resend if ack is set to false. Upon receiving an ack we go into the ack table and set the boolean to true. This protocol helped us keep track of dropped packets. Once a routers LSP table is full of all neighbors information which the received via Limited Broadcasting  it sends a Ready to the manager and continues to listen on UDP in order to forward packets or resend ACKS. Once the manger gets a ready message from each router it sleeps for a brief moment to allow the UDP ports to quiet down. It then sends a ready message to the routers to perform the minimum spanning tree algorithm. Once the forwarding table is complete the router sends another ready message to the manager. The manager then sends its first instruction packet to the start destination and waits to hear back from the destination router. Once the instruction packet reaches it destination the destination router alerts the manager then either sends a kill message to all routers in which the routers clean up all sockets or it sends the next instruction and repeats. If a kill message is sent the manager sleeps for a moment and the cleans up its sockets and then shuts down and prints a finished message to the console. Upon completion there will n about of router.out files titled routerN - routerN-1 along with a manager.out file. Inside of these file you will see each step that each router takes along with each step that the manger takes. 

——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
Sample .out files:::



--------------------------------------------------------------------------------------------------------------------------- 
Known bugs: 
1.) We tested this on the CS machines. When switching from a macbook to a CS machines timing between each process was off causing the program to fail. This will not happened if tested on a CS linux machine.
2.)This program runs on port number 15000 - 15000+n-1 and 16000 +n-1 - 1(6+n-1)000+ n-1. If these ports are not empty the program will fail. 
3.)A bug that fixed but may appear is receiving old message from the UDP once forwarding of the instruction packet begins. Program will complete but it does make output difficult to read. This bug should be fixed. 

This project took well over 60 hours it’s still not perfect but it works. 

———————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
PACKET Structures:: 
Packets to the manager are delimited by |. for example 1|8081|000. First thing in packet is signal. Second portion of packet is the message needed. Most communication from router to manager is only based on signal.
Packets manager to router are delimited by % and |. Example LSP packet to router is 1%0%2|8081|32%3|8082|4 == signal%whoYouAre%neighbor|neighborPort|neighborCost%nextNeighbor|nextNeighborPort|neightNeighborCost and so on. 

Router to router packet structure are very similar to Manager to router packet structures. Routers also use an ACK packet which looks like this 4%10$5. 4=ack signal , 10 = from who and 5 = in regards to. 


