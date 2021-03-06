/*
    To run the server.cpp file:
    g++ server.cpp -o server -std=c++11 -lpthread
    To execute:
    ./server 1
*/
#include "../../include/server.h"

void server::receivePackets(int id,int index) {
    while(1) {
        packet recvpacket;
        int count = recv(clientsSockid[id], &recvpacket, sizeof(recvpacket), 0);
        if(count < 0) {
            printf("Error on receiving message from socket %d.\n", id);
        }
        if(recvpacket.isLast==true) {
            cout << "Last packet received";
            break;   
        }
    }
}

void server::createConnection(){
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
            clilen = sizeof(clientAddr);
        }

}

void server::acceptMethod(int index) {
    // Connect to clients
    clientsSockid = new int[maxNumClients];
    for(int i=0; i<maxNumClients; i++) {
        clientsSockid[i] = accept(sockid, (struct sockaddr *)&clientAddr, &clilen);
        cout << "Inlink " << i + 1 << " connected\n";
    }

    thread clients[maxNumClients];
    for(int i=0; i<maxNumClients; i++) {
        // We can't call non static member methods directly from threads
        clients[i] = thread(&server::receivePackets, this, i,index);
    }

    for(int i=0; i<maxNumClients; i++) {
        clients[i].join();
    }
    cout << "Server " << index + 1 << "'s simulation finished\n";
}



int main(int argc, char const** argv) {
    if(argc != 2) {
        cout << "Usage ./server <index>\n";
        exit(1);
    }

    int index = stoi(argv[1]) - 1;

    server sv(index,"././samples/RED/topology/topology-server.txt");
    
    sv.createConnection();

    sv.acceptMethod(index);
    
    return 0;
}