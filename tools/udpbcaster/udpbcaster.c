#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
//#include <linux/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#define PORT 1111
#define TIMEOUTSEC 10

struct ifaddrs *ifap = NULL;


// find out whether an address is one of my 
// own addresses on one of my interfaces

char * is_this_my_address(struct sockaddr_in *pa) {
  struct ifaddrs *cp;
  for (cp = ifap;cp != NULL; cp = cp->ifa_next) {
    if (pa->sin_addr.s_addr == ((struct sockaddr_in *)(cp->ifa_addr))->sin_addr.s_addr) {
      return cp->ifa_name;
    }
  }
  return NULL;
}


int main(int argc, char *argv[]) {
  int sock;
  int yes = 1;
  struct sockaddr_in client_addr;
  struct sockaddr_in server_addr;
  struct sockaddr_in broadcast_addr;
  struct timeval polltime;
  int addr_len;
  int count;
  int ret;
  int port;
  const char *bcastmsg;

  fd_set readfd;
  char buffer[1024];

  if (argv[1] != 0) {
    port=atoi(argv[1]);
    if ((port < 1024) || (port > 16384)) {
      perror ("illegal port number");
      return -1;
    } else port = PORT;
  }

  if (argv[2] != 0) {
    bcastmsg = argv[2];
  } else {
    bcastmsg = "build-a-vent";
  }

  if (getifaddrs(&ifap) != 0) {
    perror ("getifaddrs\n");
    return -1;
  }

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("sock error\n");
    return -1;
  }

  ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&yes, sizeof(yes));
  if (ret == -1) {
    perror("setsockopt error");
    return 0;
  }


  addr_len = sizeof(struct sockaddr_in);

  memset((void*)&server_addr, 0, addr_len);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htons(INADDR_ANY);
  server_addr.sin_port = htons(port);


  memset((void*)&broadcast_addr, 0, addr_len);
  broadcast_addr.sin_family = AF_INET;
  broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
  broadcast_addr.sin_port = htons(port);

  ret = bind(sock, (struct sockaddr*)&server_addr, addr_len);
  if (ret < 0) {
    perror("bind error\n");
    return -1;
  }
  
  polltime.tv_sec = TIMEOUTSEC;
  polltime.tv_usec = 0;
  while (1) {
    FD_ZERO(&readfd);
    FD_SET(sock, &readfd);

    ret = select(sock+1, &readfd, NULL, NULL, &polltime);
    if (ret > 0) {
      if (FD_ISSET(sock, &readfd)) {
        count = recvfrom(sock, buffer+4, 1020, 0, (struct sockaddr*)&client_addr, &addr_len);
        char *ifname = is_this_my_address(&client_addr);
        if (ifname==NULL) {
          printf("\nClient connection information:\n\t IP: %s, Port: %d data %s\n", 
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
            buffer+4);
          //prepend received data with "ACK:" in-place
          memcpy(buffer, "ACK:",4);
          count = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*)&client_addr, addr_len);
        } else {
          printf("\nEcho connect information: %s data : %s\n", 
            ifname,buffer+4);
        }
      }
    }
    if (ret == 0) {
      // timeout expired, send broadcast udp message
      ret = sendto(sock, bcastmsg, strlen(bcastmsg), 0, (struct sockaddr*) &broadcast_addr, addr_len);
      printf ("broadcast.. on port %d\n",port);
      // this cumulative count-downing only works on linux, not generally on posix - see select()
      polltime.tv_sec = TIMEOUTSEC;
    }


  }
}
