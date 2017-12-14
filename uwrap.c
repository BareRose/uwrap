/*
uwrap - Portable, protocol-agnostic UDP socket wrapper, primarily designed for client-server models in applications such as games.

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring
rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software.
If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//includes
#include "uwrap.h" //file header
#ifdef _WIN32 //windows
    #define UWRAP_WINDOWS
    #include <ws2tcpip.h>
#else //unix
    #include <sys/socket.h>
    #include <netdb.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif
#include <stddef.h> //NULL

//public functions
int uwrapInit () {
    //initializes socket functionality, returns 0 on success
    #ifdef UWRAP_WINDOWS
        WSADATA WsaData;
        return (WSAStartup(MAKEWORD(2,2), &WsaData) != NO_ERROR);
    #else
        return 0;
    #endif
}
int uwrapSocket (int block, int mode, char* host, char* serv) {
    //protocol-agnostically creates a new UDP socket configured according to the given parameters
    //sockets have to be created and bound/connected all at once to allow for protocol-agnosticity
    //int: Whether to make the socket blocking or not, either UWRAP_NOBLOCK or UWRAP_BLOCK
    //int: Mode of the socket
    //  UWRAP_BIND: Bound to given address (or all interfaces if NULL) and port, e.g. for a server
    //  UWRAP_CONNECT: Only send/receive to/from given address (localhost if NULL), e.g. for a client
    //char*: Host/address as a string, can be IPv4, IPv6, etc...
    //char*: Service/port as a string, e.g. "1728" or "http"
    //returns socket handle or -1 on failure
    int sock, flags = (mode == UWRAP_BIND) ? AI_PASSIVE : 0;
    struct addrinfo* result, hint = {flags, AF_UNSPEC, SOCK_DGRAM, 0, 0, NULL, NULL, NULL};
    //get address info
    if (getaddrinfo(host, serv, &hint, &result) != 0) return -1; //return -1 on error
    //create socket
    sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    #ifdef UWRAP_WINDOWS
        if (sock == INVALID_SOCKET) return -1; //return -1 on error
    #else
        if (sock == -1) return -1; //return -1 on error
    #endif
    //make sure IPV6_ONLY is disabled
    if (result->ai_family == AF_INET6) {
        int no = 0;
        setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (void*)&no, sizeof(no));
    }
    //bind or connect
    if ((mode == UWRAP_BIND) ? bind(sock, result->ai_addr, result->ai_addrlen) : connect(sock, result->ai_addr, result->ai_addrlen)) {
        //close socket and return -1 on error
        uwrapClose(sock);
        return -1;
    }
    //free address info
    freeaddrinfo(result);
    //set non-blocking if needed
    if (block == UWRAP_NOBLOCK) {
        #ifdef UWRAP_WINDOWS
        DWORD no_block = 1;
        if (ioctlsocket(sock, FIONBIO, &no_block) != 0) {
        #else
        if (fcntl(sock, F_SETFL, O_NONBLOCK, 1) == -1) {
        #endif
            //close socket and return -1 on error
            uwrapClose(sock);
            return -1;
        }
    }
    //return socket handle
    return sock;
}
int uwrapSendTo (int sock, struct uwrap_addr* addr, char* data, int data_size) {
    //uses the given socket to send given data (pointer + size) to the given uwrap_addr (as given by uwrapReceiveFrom)
    //primarily intended for a server (i.e. UWRAP_BIND) socket to respond to a client after uwrapReceiveFrom
    //returns 0 on success, non-zero on failure
    return (sendto(sock, data, data_size, 0, (struct sockaddr*)addr, sizeof(struct uwrap_addr)) != data_size);
}
int uwrapReceiveFrom (int sock, struct uwrap_addr* addr, char* data, int data_size) {
    //receives a packet using given socket, writing sender's address and sent data to the given pointers
    //intended for a server (i.e. UWRAP_BIND) socket to receive messages from any clients and possibly respond
    //returns the size of the received packet, or -1 or 0 instead if there are no packets
    #ifdef UWRAP_WINDOWS
        int addr_size = sizeof(struct uwrap_addr);
    #else
        socklen_t addr_size = sizeof(struct uwrap_addr);
    #endif
    //return
    return recvfrom(sock, data, data_size, 0, (struct sockaddr*)addr, &addr_size);
}
int uwrapSend (int sock, char* data, int data_size) {
    //uses the given socket (must be UWRAP_CONNECT) to send given data (pointer + size) to destination assigned on creation
    //returns 0 on success, non-zero on failure
    return (send(sock, data, data_size, 0) != data_size);
}
int uwrapReceive (int sock, char* data, int data_size) {
    //receives a packet using given socket (must be UWRAP_CONNECT), only receives packets from destination assigned on creation
    //returns the size of the received packet, or -1 or 0 instead if there are no packets
    return recv(sock, data, data_size, 0);
}
int uwrapSelect (int sock, double timeout) {
    //waits either until given socket has new packet(s) to uwrapReceive or given time (in seconds) has passed,
    //returns 1 if new packet(s) are available, 0 if timeout was reached, and -1 on error
    fd_set set; struct timeval time;
    //fd set
    FD_ZERO(&set);
    FD_SET(sock, &set);
    //timeout
    time.tv_sec = timeout;
    time.tv_usec = (timeout - time.tv_sec)*1000000;
    //return
    return select(sock+1, &set, NULL, NULL, &time);
}
void uwrapClose (int sock) {
    //closes the given socket
    #ifdef UWRAP_WINDOWS
        closesocket(sock);
    #else
        close(sock);
    #endif
}
void uwrapTerminate () {
    //terminates socket functionality
    #ifdef UWRAP_WINDOWS
        WSACleanup();
    #endif
}
