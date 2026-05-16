#ifndef LINK_H
#define LINK_H

#include "pus.h"

#define OBC_HOST "127.0.0.1"
#define OBC_PORT 9000

// Wire format: 38 bytes fixed
//   [0]     version
//   [1]     type (0=TM, 1=TC)
//   [2]     service
//   [3]     subtype
//   [4-5]   length (big-endian uint16)
//   [6-37]  data (32 bytes, zero-padded)

int  link_obc_init(void);          // OBC: socket+bind+listen, returns server fd (call once)
int  link_obc_accept(int server_fd); // OBC: accept next ground connection, returns client fd
int  link_ground_connect(void);    // Ground: connect to OBC, returns socket fd
int  link_send_pkt(int fd, const pus_packet_t* pkt);
int  link_recv_pkt(int fd, pus_packet_t* pkt);
void link_close(int fd);

#endif
