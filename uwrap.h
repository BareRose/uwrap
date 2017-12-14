/*
uwrap - Portable, protocol-agnostic UDP socket wrapper, primarily designed for client-server models in applications such as games.

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring
rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software.
If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef UWRAP_H
#define UWRAP_H

//constants
#define UWRAP_NOBLOCK 0
#define UWRAP_BLOCK 1
#define UWRAP_BIND 0
#define UWRAP_CONNECT 1

//structs
struct uwrap_addr {
    char data[128]; //enough space to hold any kind of address
};

//public functions
int uwrapInit();
    //initializes socket functionality, returns 0 on success
int uwrapSocket(int, int, char*, char*);
    //protocol-agnostically creates a new socket configured according to the given parameters
    //sockets have to be created and bound/connected all at once to allow for protocol-agnosticity
    //int: Whether to make the socket blocking or not, either UWRAP_NOBLOCK or UWRAP_BLOCK
    //int: Mode of the socket
    //  UWRAP_BIND: Bound to given address (or all interfaces if NULL) and port, e.g. for a server
    //  UWRAP_CONNECT: Only send/receive to/from given address (localhost if NULL), e.g. for a client
    //char*: Host/address as a string, can be IPv4, IPv6, etc...
    //char*: Service/port as a string, e.g. "1728" or "http"
    //returns socket handle or -1 on failure
int uwrapSendTo(int, struct uwrap_addr*, char*, int);
    //uses the given socket to send given data (pointer + size) to the given uwrap_addr (as given by uwrapReceiveFrom)
    //primarily intended for a server (i.e. UWRAP_BIND) socket to respond to a client after uwrapReceiveFrom
    //returns 0 on success, non-zero on failure
int uwrapReceiveFrom(int, struct uwrap_addr*, char*, int);
    //receives a packet using given socket, writing sender's address and sent data to the given pointers
    //intended for a server (i.e. UWRAP_BIND) socket to receive messages from any clients and possibly respond
    //returns the size of the received packet, or -1 or 0 instead if there are no packets
int uwrapSend(int, char*, int);
    //uses the given socket (must be UWRAP_CONNECT) to send given data (pointer + size) to destination assigned on creation
    //returns 0 on success, non-zero on failure
int uwrapReceive(int, char*, int);
    //receives a packet using given socket (must be UWRAP_CONNECT), only receives packets from destination assigned on creation
    //returns the size of the received packet, or -1 or 0 instead if there are no packets
int uwrapSelect(int, double);
    //waits either until given socket has new packet(s) to receive or given time (in seconds) has passed,
    //returns 1 if new packet(s) are available, 0 if timeout was reached, and -1 on error
void uwrapClose(int);
    //closes the given socket
void uwrapTerminate();
    //terminates socket functionality

#endif