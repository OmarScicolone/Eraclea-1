#include "link.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define PKT_SIZE 38   // 6-byte header + 32-byte data

static void serialize(const pus_packet_t* pkt, uint8_t* buf)
{
    buf[0] = pkt->header.version;
    buf[1] = pkt->header.type;
    buf[2] = pkt->header.service;
    buf[3] = pkt->header.subtype;
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
    pkt->header.length  = ((uint16_t)buf[4] << 8) | buf[5];
    if (pkt->header.length > 32)
        pkt->header.length = 32;
    memcpy(pkt->data, buf + 6, 32);
}

int link_obc_listen(void)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("[LINK] socket");
        return -1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(OBC_PORT);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("[LINK] bind");
        close(server_fd);
        return -1;
    }
    if (listen(server_fd, 1) < 0) {
        perror("[LINK] listen");
        close(server_fd);
        return -1;
    }

    int client_fd = accept(server_fd, NULL, NULL);
    close(server_fd);
    return client_fd;
}

int link_ground_connect(void)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("[LINK] socket");
        return -1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = inet_addr(OBC_HOST);
    addr.sin_port        = htons(OBC_PORT);

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
    ssize_t n = send(fd, buf, PKT_SIZE, 0);
    return (n == PKT_SIZE) ? 0 : -1;
}

int link_recv_pkt(int fd, pus_packet_t* pkt)
{
    uint8_t buf[PKT_SIZE];
    ssize_t total = 0;
    while (total < PKT_SIZE) {
        ssize_t n = recv(fd, buf + total, PKT_SIZE - total, 0);
        if (n <= 0)
            return -1;
        total += n;
    }
    deserialize(buf, pkt);
    return 0;
}

void link_close(int fd)
{
    close(fd);
}
