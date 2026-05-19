#include "link.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

// Fixed on-wire size for every PUS packet: 6-byte header + 32-byte payload
#define PKT_SIZE 38

static void serialize(const pus_packet_t* pkt, uint8_t* buf)
{
    buf[0] = pkt->header.version;
    buf[1] = pkt->header.type;
    buf[2] = pkt->header.service;
    buf[3] = pkt->header.subtype;
    // 16-bit length sent big-endian (most significant byte first)
    buf[4] = (uint8_t)(pkt->header.length >> 8);
    buf[5] = (uint8_t)(pkt->header.length & 0xFF);
    memset(buf + 6, 0, 32);
    uint16_t len = pkt->header.length > 32 ? 32 : pkt->header.length;
    if (len > 0)
        memcpy(buf + 6, pkt->data, len);
}

static void deserialize(const uint8_t* buf, pus_packet_t* pkt)
{
    pkt->header.version = buf[0];
    pkt->header.type    = buf[1];
    pkt->header.service = buf[2];
    pkt->header.subtype = buf[3];
    // reconstruct 16-bit length from big-endian bytes
    pkt->header.length  = ((uint16_t)buf[4] << 8) | buf[5];
    if (pkt->header.length > 32)
        pkt->header.length = 32;
    memcpy(pkt->data, buf + 6, 32);
}

int link_obc_init(void)
{
    // AF_INET = IPv4
    // SOCK_STREAM = TCP
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("[LINK] socket");
        return -1;
    }

    // SO_REUSEADDR: allows reusing the port immediately after a process restart,
    // bypassing the kernel's TIME_WAIT timeout (~60s)
    // SOL_SOCKET = socket-level option (as opposed to protocol-specific levels like IPPROTO_TCP)
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("[LINK] setsockopt");
        close(server_fd);
        return -1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;          // accept connections on any local network interface
    addr.sin_port        = htons(OBC_PORT);     // htons: convert port from host byte order (little-endian) to network byte order (big-endian)

    // bind: reserves the port — no other process can use it from this point
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("[LINK] bind");
        close(server_fd);
        return -1;
    }
    // listen: marks the socket as passive; backlog=1 since only one ground station is expected
    if (listen(server_fd, 1) < 0) {
        perror("[LINK] listen");
        close(server_fd);
        return -1;
    }

    return server_fd;
}

// BLOCKING: suspends the thread until a ground station connects.
// Returns a new fd dedicated to that connection; server_fd stays open for future connections.
int link_obc_accept(int server_fd)
{
    return accept(server_fd, NULL, NULL);
}

int link_ground_connect(void)
{
    // AF_INET = IPv4
    // SOCK_STREAM = TCP
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("[LINK] socket");
        return -1;
    }

    // sockaddr_in = structure that holds destination IP address and port
    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;                           // family = IPv4
    addr.sin_addr.s_addr = inet_addr(OBC_HOST);               // inet_addr: converts "127.0.0.1" string to binary uint32
    addr.sin_port        = htons(OBC_PORT);                   // htons: converts port number to network byte order (big-endian)

    // BLOCKING: attempts the TCP handshake with OBC_HOST:OBC_PORT
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("[LINK] connect");
        close(fd);
        return -1;
    }
    return fd;
}

int link_send_pkt(int fd, const pus_packet_t* pkt)
{
    uint8_t buf[PKT_SIZE];
    serialize(pkt, buf);
    ssize_t total = 0;
    // Loop required: TCP may fragment the buffer and send only part of it at a time
    while (total < PKT_SIZE) {
        // BLOCKING: suspends until the kernel has space in the send buffer
        ssize_t n = send(fd, buf + total, PKT_SIZE - total, 0);
        if (n <= 0)  // n==0: connection closed; n<0: error
            return -1;
        total += n;
    }
    return 0;
}

int link_recv_pkt(int fd, pus_packet_t* pkt)
{
    uint8_t buf[PKT_SIZE];
    ssize_t total = 0;
    // Loop required: TCP may deliver data in chunks smaller than the expected packet
    while (total < PKT_SIZE) {
        // BLOCKING: suspends until data arrives from the sender
        ssize_t n = recv(fd, buf + total, PKT_SIZE - total, 0);
        if (n <= 0)  // n==0: connection closed; n<0: error
            return -1;
        total += n;
    }
    deserialize(buf, pkt);
    return 0;
}

void link_close(int fd)
{
    // shutdown before close: sends TCP FIN to notify the peer no more data will be sent
    // SHUT_RDWR = close both read and write sides of the connection
    shutdown(fd, SHUT_RDWR);
    close(fd);
}
