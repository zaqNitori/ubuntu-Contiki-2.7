
#ifndef RPL_LOC_H
#define RPL_LOC_H
#include "net/ipv6/uip.h"
#include "stdlib.h"
#include "stdio.h"

//test for mass_spring_model_localization 
#define P_0      -63.17164
#define gama     21.7409
#define STD      1.62 // cc2538dk
#define LOG_e10  2.302585092994L // log_e (10)

typedef enum 
{
	Location_Info,
	MASS_SPRING_REQUEST,
	MASS_SPRING_CALLBACK,
	Location_Info_From_Client
}rpl_loc_t;

typedef enum 
{
	LOCATION_X,
	LOCATION_Y,
	MSG_TYPE,
	BC_TIME
}rpl_loc_info_t;

typedef struct
{
  rpl_loc_t Msg_Type;
  int x;	
  int y;
  int bc_time;	
  uip_ipaddr_t addr;
}rpl_loc_msg_t;

uip_ipaddr_t BRaddr;
int loc_x;
int loc_y;
int bc_time;

void ConvertToMsg(char *buf, int x, int y, int type, int bc_time)
{
	sprintf( buf, "%d,%d,%d,%d", x, y, type, bc_time);
}

void split(char **arr, char *str, const char *del) 
{
	char *s = strtok(str, del);

	while(s != NULL) 
	{
		*arr++ = s;
		s = strtok(NULL, del);
	}
}

void GetMsg(char *str, int *x, int *y, int *type, int *bc_time)
{
	char *arr[4];
	split( arr, str, ",");
	*x = atoi(*arr+LOCATION_X);
	*y = atoi(*(arr+LOCATION_Y));
	*type = atoi(*(arr+MSG_TYPE));
	*bc_time = atoi(*(arr+BC_TIME));
}




#endif /* RPL_LOC_H */
