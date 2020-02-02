#include <stdio.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include "parserequest.h"
#include "buildheaders.h"
#include <sys/sendfile.h>

// Set port to anything, but if it's a low port (1024 or under), the program must be run as root to bind.
const int PORT = 21988;

int getSock(){
    // create a socket and return the file descriptor integer. If it fails it will return a negative integer.
    return socket(AF_INET, SOCK_STREAM, 0); // IP socket, virtual circuit service
}

void closeSock(int sock){
    // Close the socket when no longer needed
    printf("\n\n\n--------------    Connection Closed    ---------------------\n");
    close(sock);
}

int bindSock(int sock, struct sockaddr_in address, int len){
    // Set up address structure ready to bind
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = htonl(INADDR_ANY); 
    address.sin_port = htons(PORT); 
    // bind socket to address and return status code
    return bind(sock,(struct sockaddr *)&address,sizeof(address));
}

int listenSock(int sock){
    // Listen to socket with maximum 5 queued connections.
    return listen(sock, 5);
}

int acceptSock(int sock, struct sockaddr_in address, int len){
    // Accept incoming connections on socket.
    printf("\n\n\n++++++++++++++++    New Connection    ++++++++++++++++++++++\n\n");
    return accept(sock, (struct sockaddr *)&address, (socklen_t*)&len);
}

int readSock(int sock, char buff[]){
    // Use read system call to read bytes received on socket into a buffer.
    int int_r = read(sock, buff, 2048);
    if (int_r < 0) {
        printf("No bytes have been read");
        return 0;
    } else {
        printf("Received request: %s\n", buff);
        return 1;
    }
}

void respondHTTP(int sock){
    char rcv_buffer[2048] = {0};
    char path[256];
    // Read request into rcv_buffer
    readSock(sock, rcv_buffer);
    // Use request to work out path of file to respond with
    getFileToSend(rcv_buffer, path);
    // Work out the correct header data. 
    char headers[8192];
    memset(headers, 0, sizeof(headers));
    buildHeaders(headers, path);
    printf("Headers:\n\n%s\n", headers);
    // Send headers
    printf("Sending headers... \n");
    write(sock, headers, strlen(headers));
    printf("Sent headers.\nSending File...\n");
    int fd = open(path, O_RDONLY);
    if (fd > 2){ // only send file if it exists, if it doesn't, headers will have error 404 html embdedded so no file needs to be sent.
        sendfile(sock, fd, NULL, 500000);
        printf("Sent File.\n");
    } else {
        printf("Could not send file.\n");
    }
    closeSock(sock);
}

int main(){
    // Create Socket
    int sock = getSock(); // sock is an integer file descriptor to identify our socket
    if (sock < 0) { // Socket FD is negative if there has been an error
        printf("Failed to create Socket.\n");
        return 0;
    }
    printf("Created socket with fd %d.\n", sock);

    // Bind Socket
    struct sockaddr_in address;
    int address_len = sizeof(address);
    int bind_sucess = bindSock(sock, address, address_len);
    if (bind_sucess < 0) {
        printf("Failed to bind Socket. Error code %d\n", bind_sucess); // Error code -1 received when not running as root and attempting to bind to low number port!
        closeSock(sock);
        return 0;
    }
    printf("Bind success.\n");

    // Listen on Socket
    int listen_success = listenSock(sock);
    if (listen_success < 0) {
        printf("Failed to listen on socket.\n");
        closeSock(sock);
        return 0;
    }

    int newsock;
    while(1){
        newsock = acceptSock(sock, address, address_len); // a new socket is opened and then closed for each request-response pair.
        if (newsock < 0){
            printf("Socket accept error. Exiting.\n");
            return 0;
        }
        respondHTTP(newsock);
    }
    return 0;
}
