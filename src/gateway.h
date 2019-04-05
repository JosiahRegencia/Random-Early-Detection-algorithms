#include <stdio.h>
#include <cstdlib>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>
#include <netdb.h>
#include <thread>
#include <arpa/inet.h>
#include <chrono>
#include <queue>
#include <mutex>
#include <iostream>
#include <fstream>
#include <map>
#include <set>

using namespace std;

// Class to represent a single packet
class packet {
public:
    bool isLast;
    int destPortNo;
    char charPayload;
    int clientNum;

    packet() {
        isLast=false;
    }    
};

class gateway {
public:
    int sockid, maxNumClients, simTime, portNo, receivedLastPackets;
    struct sockaddr_in addrport, clientAddr;
    socklen_t clilen;
    int *clientsSockid;
    queue<packet*> Queue;
    mutex mtx;                  // For RED() packet synchronization
    mutex mtx2;                 // For bufferPackets synchronization
    mutex mtx3;                 // For receivedLastPacket variable's synchronization
    vector<packet*> bufferPackets;
    map<int, int> portId;       // For mapping dest port nos to outlink's port no.
    map<int, int> mp;
    ofstream fout;

    // RED Algorithm's parameters
    double avg = 0;             // Average queue length
    int count = -1;             // Count of packets since last probabilistic drop
    double wq = 0.003;          // Queue weight; standard value of 0.002 for early congestion detection
    int minThreshold = 5, maxThreshold = 17;
    double maxp = 0.02;         // Maximum probability of dropping a packet; standard value of 0.02
    double pb = 0;              // Probability of dropping a packet
    time_t qTime;               // Time since the queue was last idle

    // Constructor for Gateway object
    gateway(int indexNo, int simulationTime, string traffic) {
        simTime = simulationTime;

        // Reading topology file for getting the info of the gateway
        ifstream fin;
        string fileName = "./topology/topology-gateway.txt";
        fin.open(fileName);
        string line;
        while(getline(fin, line)) {
            if(line == ("#" + to_string(indexNo + 1))) {
                break;
            }
        }

        fin >> portNo >> maxNumClients;
        cout << "port no = " << portNo << " maxNumClients = " << maxNumClients << endl;
        int szMap, e1, e2;
        fin >> szMap;

        for(int i=0; i<szMap; i++) {
            fin >> e1 >> e2;
            portId[e1] = e2;
        }

        fin.close();
        // Topology reading complete

        // Creating socket for the gateway
        sockid = socket(PF_INET, SOCK_STREAM, 0);
        if(sockid < 0) {
            printf("Socket could not be opened.\n");
        }
        addrport.sin_family = AF_INET;
        addrport.sin_port = htons(portNo);
        addrport.sin_addr.s_addr = htonl(INADDR_ANY);
        int t = 1;
        setsockopt(sockid, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(int));

        if(bind(sockid, (struct sockaddr *)&addrport, sizeof(addrport)) < 0) {
            printf("Error in binding socket\n");
        } else {
            // Socket is bound
            int status = listen(sockid, maxNumClients);
            if(status < 0) {
                printf("Error in listening.\n");
            }
            // servlen = sizeof(serverAddr);
            clilen = sizeof(clientAddr);
        }

        fileName = "./samples/log-" + to_string(indexNo + 1) + ".txt";
        fout.open(fileName);
        // NOTE: Writing traffic level to log file
        // for plotter to read 
        fout << traffic << endl;

        // Starting simulation for this gateway 
        acceptMethod(indexNo, traffic);

        fout.close(); 
    }

    /* Methods for the Gateway */
    // Method for simulating RED algorithm on a packet
    void red(packet* packet);

    // Deques the queue and also sends the packet to the corresponding outlink
    void dequeQueue();

    // This method simulates each burst by calling red() on each packet of burst
    void simulateRED();

    // The gateway creates thread for each client and calls this method
    void receivePackets(int id);

    // This method accepts connection from all clients 
    // and creates connection to the outlinks as well
    // The simulateRED() method is called from this method
    void acceptMethod(int index, string traffic);

    // Helper method to show the contents of the queue
    void showq(queue<char> q);
};