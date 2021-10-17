
#ifndef RPL_LOC_H
#define RPL_LOC_H
#include "net/ipv6/uip.h"
#include "stdlib.h"
#include "stdio.h"

//test for mass_spring_model_localization 
#define P_0      -62.3204
#define gama     26.575
#define STD      1.62 // cc2538dk
#define LOG_e10  2.302585092994L // log_e (10)

typedef enum 
{
	Location_Info,
	MASS_SPRING_REQUEST,
	MASS_SPRING_CALLBACK,
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
	*x = atoi(*arr);
	*y = atoi(*(arr+1));
	*type = atoi(*(arr+2));
	*bc_time = atoi(*(arr+3));
}




#endif /* RPL_LOC_H */
