
#ifndef RPL_LOC_H
#define RPL_LOC_H
#include "net/ipv6/uip.h"

//test for mass_spring_model_localization 
#define P_0      -44.06//-44.06
#define gama     24.13//24.13
#define STD      1.62 // cc2538dk
#define LOG_e10  2.302585092994L // log_e (10)

typedef enum 
{
	Location_Info,
	MASS_SPRING_REQUEST_INFO,
	Location_Info_From_Client
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
