
#ifndef RPL_LOC_H
#define RPL_LOC_H
#include "net/ipv6/uip.h"
typedef enum 
{
	Location_Info
}rpl_loc_t;

typedef struct
{
  rpl_loc_t Msg_Type;
  int x;	
  int y;
  int bc_time;	
  uip_ipaddr_t addr;
}rpl_loc_msg_t;

uip_ipaddr_t BRaddr;
short start = 0;
int loc_x;
int loc_y;
int bc_time;

#endif /* RPL_LOC_H */
