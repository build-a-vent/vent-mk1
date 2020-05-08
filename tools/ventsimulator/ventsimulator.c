#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#define DFTPORT 1111
#define TIMEOUTSEC 10

/* args : 1 : port number
 *        2 : vent name
 *        3 : vent mac (3 bytes)
 */

#include "parson.h"
//
// JSON representation of vent status

JSON_Value *root_value = NULL;
JSON_Object *root_object = NULL;


// find out whether an address is one of my 
// own addresses on one of my interfaces

struct ifaddrs *ifap = NULL;
char * is_this_my_address(struct sockaddr_in *pa) {
  struct ifaddrs *cp;
  for (cp = ifap;cp != NULL; cp = cp->ifa_next) {
    if (pa->sin_addr.s_addr == ((struct sockaddr_in *)(cp->ifa_addr))->sin_addr.s_addr) {
      return cp->ifa_name;
    }
  }
  return NULL;
}


struct s_ptable {
  char*   name;
  int32_t initval;
  uint8_t editable;
} json_inittab[] = {
  { "c_pip",    35, 1 }, // config max inspir press
  { "c_lip",    10, 1 }, // config lowest inspir press
  { "c_pep",    22, 1 }, // config peak exspir press
  { "c_lep",    10, 1 }, // config lowest exspir press
  { "c_flair", 750 ,1 }, // config flow rate air ml/sec
  { "c_flo2",  750, 1 }, // config flow rate o2 ml/sec
  { "c_airt",  330, 1 }, // config air time ms (approx 240ml)
  { "c_o2t",   330, 1 }, // config o2 time ms (approx 240ml)
  { "c_inspt",1500, 1 }, // config inspir. time(before releasing outflow)
  { "c_cyclt",3000, 1 }, // cyle time (20 cycles / min)
  { "c_wtemp",  38, 1 }, // config herater water temp
  { "a_eep",    17, 0 }, // actual end exspirat. pressure last cycle
  { "a_wtemp",  30, 0 }, // config herater water temp
  { "fio2_pct", 40, 1 },
  { "b_rat",    55, 1 },
  { "cyc_min",  24, 1 },
  { "tidal_ml", 500,1 },
  { NULL,        0, 0 }
};


const struct s_ptable * get_ptable_by_name(const char *s) {
  for (uint32_t i=0;json_inittab[i].name != NULL; ++i) {
    if (!strcmp(json_inittab[i].name,s))
      return &(json_inittab[i]);
  }
  return NULL;
}


void initialize_vent(JSON_Object *pRoot) {
  for (uint8_t i=0; json_inittab[i].name!=NULL;++i) {
    json_object_set_number(pRoot, json_inittab[i].name,json_inittab[i].initval);
  }
}

void json_copy_string_if_exists(JSON_Object *pD, const JSON_Object *pS, char *name) {
  if(json_object_has_value (pS,name)) {
    const char *s = json_object_get_string(pS,name);
    json_object_set_string(pD,name,s);
  }
}

void json_copy_number_if_exists(JSON_Object *pD, const JSON_Object *pS, char *name) {
  if(json_object_has_value (pS,name)) {
    int64_t value = json_object_get_number(pS,name);
    json_object_set_number(pD,name,value);
  }
}


void add_value_to_obj(JSON_Object *pD, const char *name, JSON_Value *pS) {
  JSON_Value_Type p = json_value_get_type(pS);
  switch (p) {
    case JSONNumber:
      if (json_object_set_number(pD,name,json_value_get_number(pS)) != JSONSuccess) {
        printf ("failed to set numeric value for %s\n",name);
      }
      break;
    case JSONString:
      if (json_object_set_string(pD,name,json_value_get_string(pS)) != JSONSuccess) {
        printf ("failed to set string value for %s\n",name);
      }
      break;
    default:
      printf ("unexpected json type %d for %s\n",p,name);
  }
}

void merge_to_object(JSON_Object *pD, JSON_Object *pS) {
  int32_t n = json_object_get_count(pS);

  for (int32_t i = 0; i<n;++i) {
    const char *name = json_object_get_name(pS,i);
    JSON_Value *pcV = json_object_get_value_at(pS,i);
    if (! pcV) {
      printf ("get_value failed for %s\n",name);
    } else {
      add_value_to_obj(pD,name,pcV);
    }
  }

}

uint32_t udprecord_process(char *buffer, uint32_t bufsize) { 

  JSON_Value  * pV = json_parse_string(buffer);
  JSON_Object * pO = json_value_get_object (pV);

  JSON_Value *reply_value = json_value_init_object();
  JSON_Object *reply_object = json_value_get_object(reply_value);
 
  if(json_object_has_value (pO,"cmd")) {
    const char * cmd = json_object_get_string(pO,"cmd");
    json_object_set_string(reply_object, "req", cmd);

    if (!strcmp(cmd,"scan")) {
      json_object_set_string(reply_object, "cmd", "stat");
      json_copy_number_if_exists(reply_object,pO,"seq");      
      merge_to_object(reply_object,root_object);
    }
    if (!strcmp(cmd,"save")) {
      json_object_set_string(reply_object, "cmd", "AKK");
      json_copy_number_if_exists(reply_object,pO,"seq");      
      int32_t tparm = json_object_get_count(pO);
      for (int32_t i = 0; i<tparm;++i) {
        const char *name = json_object_get_name(pO,i);
        const struct s_ptable * pTab = get_ptable_by_name(name);
        if (NULL != pTab) {
          if (pTab->editable) {
            JSON_Value * pV = json_object_get_value_at(pO,i);
            add_value_to_obj(root_object,name,pV);
            add_value_to_obj(reply_object,name,pV);
          }
        }
      }
    }
  } else {
    json_object_set_string(reply_object, "cmd", "nak");
  }

  char * sss = json_serialize_to_string(reply_value);
  strcpy (buffer,sss);
  printf ("result %s \n",sss);

  json_free_serialized_string(sss);
  json_value_free(pV);
  json_value_free(reply_value);
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
  int port = DFTPORT;
  const char *bcastmsg;

  root_value = json_value_init_object();
  root_object = json_value_get_object(root_value);



  fd_set readfd;
  char buffer[2048];

  if ((argc > 0) && (argv[1] != 0)) {
    port=atoi(argv[1]);
    if ((port < 1024) || (port > 16384)) {
      perror ("illegal port number");
      return -1;
    }
  }

  /* choose a name */

  char * ventname = "vent-1";
  if ((argc > 1) && (argv[2] != 0)) {
    ventname = argv[2];
  } 
  json_object_set_string(root_object, "name", ventname);
  
  /* dice a mac */
  int ventmac = 47113013;
  if (argc > 2) {
    ventmac ^= atoi(argv[3]);
  }
  
  uint8_t *pm = (uint8_t*)&ventmac;
  char ventmacbuf[50];
  sprintf(ventmacbuf,"%02hx:%02hx:%02hx",pm[0],pm[1],pm[2]);
  json_object_set_string(root_object, "mac", ventmacbuf);
  
  
  initialize_vent(root_object);
  //
  // have a list of my own interfaces at hand
  // in order to identify my own messages
  //
  if (getifaddrs(&ifap) != 0) {
    perror ("getifaddrs\n");
    return -1;
  }
  // get the socket
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("sock error\n");
    return -1;
  }


  addr_len = sizeof(struct sockaddr_in);

  memset((void*)&server_addr, 0, addr_len);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htons(INADDR_ANY);
  server_addr.sin_port = htons(port);

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
        count = recvfrom(sock, buffer, 1536, 0, (struct sockaddr*)&client_addr, &addr_len);
        char *ifname = is_this_my_address(&client_addr);
        if (ifname==NULL) { // request came from another station on the net
          buffer[count]=0;
          printf("\nClient connection information:\n\t IP: %s, Port: %d data %s\n", 
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
            buffer);
          //
          // fixme distinguish btw uni- and broadcast
          //
          udprecord_process(buffer,sizeof(buffer));
          count = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*)&client_addr, addr_len);
        } else {
          printf("\nEcho connect information: %s data : %s\n", 
            ifname,buffer+4);
        }
      }
    }
    if (ret == 0) {
      // timeout expired, print current json status to console
      // this cumulative count-downing only works on linux, not generally on posix - see select()
    char * serialized_string = json_serialize_to_string(root_value);
    printf ("json status = %s \n",serialized_string);
    json_free_serialized_string(serialized_string);
      polltime.tv_sec = TIMEOUTSEC;
    }


  }
}
