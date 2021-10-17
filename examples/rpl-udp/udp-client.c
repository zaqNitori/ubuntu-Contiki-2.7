#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "net/routing/rpl-location/rpl-loc.h"
#include <math.h>

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678
#define UDP_BR_PORT 3333

#define SEND_MASS_SPRING_REQUEST_INFO_INTERVAL (25 * CLOCK_SECOND)
#define SEND_LOCATION_INFO_TO_SERVER_INTERVAL (30 * CLOCK_SECOND)

static struct simple_udp_connection udp_connCS;
static struct simple_udp_connection udp_connCBR;
static struct simple_udp_connection udp_connCC;
static int begin = 0;
/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static double mass_spring_model_localization(radio_value_t rss)
{
	double f=0;
	double distance;
	f = ((double)P_0 - (double)rss) / (double)(gama);
	distance = expf( f * (double)LOG_e10);
	return distance;
}
/*---------------------------------------------------------------------------*/
static double GetDistance(double dx, double dy)
{
	double dis = sqrt(pow(((loc_x * 1.0 / 1000) - dx),2) + pow(((loc_y * 1.0 / 1000) - dy),2));
	return dis;
}
/*---------------------------------------------------------------------------*/
static void 
Do_Mass_Spring_Model_Localization(double dx, double dy)
{
	radio_value_t v;
	NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_RSSI,&v);
	LOG_INFO_("Received RSSI => %d\nFrom ",v);
	double rssi_dis = mass_spring_model_localization(v);
	double location_dis = GetDistance(dx, dy);

	double diff_x = ((loc_x * 1.0 / 1000) - dx) * rssi_dis / location_dis;
	double diff_y = ((loc_y * 1.0 / 1000) - dy) * rssi_dis / location_dis;

	loc_x = (dx + diff_x) * 1000;
	loc_y = (dy + diff_y) * 1000;
}
/*---------------------------------------------------------------------------*/
static void 
Pass_On_Location_Information()
{
	char buf[30];
	uip_ipaddr_t addr;
	
	ConvertToMsg(buf, loc_x, loc_y, Location_Info, bc_time);
	uip_create_linklocal_allnodes_mcast(&addr);
	simple_udp_sendto(&udp_connCC, &buf, sizeof(buf), &addr);	
}
/*---------------------------------------------------------------------------*/
static void
udp_rx_CBR_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{

}
/*---------------------------------------------------------------------------*/
static void
udp_rx_CS_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
	int msg_x, msg_y, msg_type, msg_bc;
	GetMsg((char*)data, &msg_x, &msg_y, &msg_type, &msg_bc);
	double dx = msg_x * 1.0 / 1000;
	double dy = msg_y * 1.0 / 1000;	

	if (msg_type == Location_Info)
	{
		if (msg_bc < bc_time)
		{
			bc_time = ++msg_bc;		

			Do_Mass_Spring_Model_Localization(dx, dy);
			LOG_INFO_6ADDR(sender_addr);
			LOG_INFO_("\n");

			LOG_INFO_("My bc_time is %d\n", bc_time);
			LOG_INFO_("My x, y is %d, %d\n", loc_x, loc_y);

			Pass_On_Location_Information();
		}
	}

}
/*---------------------------------------------------------------------------*/
static void
udp_rx_CC_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
	int msg_x, msg_y, msg_type, msg_bc;
	GetMsg((char*)data, &msg_x, &msg_y, &msg_type, &msg_bc);
	double dx = msg_x * 1.0 / 1000;
	double dy = msg_y * 1.0 / 1000;	

	if (msg_type == Location_Info)
	{
		if (msg_bc < bc_time)
		{
			bc_time = ++msg_bc;			

			Do_Mass_Spring_Model_Localization(dx, dy);
			LOG_INFO_6ADDR(sender_addr);
			LOG_INFO_("\n");

			LOG_INFO_("My bc_time is %d\n", bc_time);
			LOG_INFO_("My x, y is %d, %d\n", loc_x, loc_y);

			Pass_On_Location_Information();
		}
	}
	else if (msg_type == MASS_SPRING_REQUEST)
	{
		char buf[30];
		LOG_INFO_("Received MASS_SPRING_REQUEST From ");
		LOG_INFO_6ADDR(sender_addr);
		LOG_INFO_("\n");
		ConvertToMsg(buf, loc_x, loc_y, MASS_SPRING_CALLBACK, bc_time);
		simple_udp_sendto(&udp_connCC, &buf, sizeof(buf), sender_addr);
	}
	else if (msg_type == MASS_SPRING_CALLBACK)
	{
		LOG_INFO_("Received MASS_SPRING_CALLBACK From ");
		LOG_INFO_6ADDR(sender_addr);
		LOG_INFO_("\n");
		Do_Mass_Spring_Model_Localization(dx, dy);
	}
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
	static struct etimer periodic_timer;
	static struct etimer mass_spring_timer;
	char buf[30];
	uip_ipaddr_t dest_ipaddr;
	uip_ipaddr_t addr;
	if(!begin)
	{
		bc_time = 100;
		loc_x = (random_rand() % 51) * 1000;
		loc_y = (random_rand() % 51) * 1000;
		begin = 1;
		LOG_INFO_("%d,%d\n",loc_x,loc_y);
	}

	PROCESS_BEGIN();

	/* Initialize UDP connection */
	simple_udp_register(&udp_connCS, UDP_CLIENT_PORT, NULL,
		              UDP_SERVER_PORT, udp_rx_CS_callback);
	simple_udp_register(&udp_connCBR, UDP_CLIENT_PORT, NULL,
		              UDP_BR_PORT, udp_rx_CBR_callback);
	simple_udp_register(&udp_connCC, UDP_CLIENT_PORT, NULL,
		              UDP_CLIENT_PORT, udp_rx_CC_callback);


	etimer_set(&periodic_timer, SEND_LOCATION_INFO_TO_SERVER_INTERVAL);
	etimer_set(&mass_spring_timer, SEND_MASS_SPRING_REQUEST_INFO_INTERVAL);
	while(1) 
	{
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer) || etimer_expired(&mass_spring_timer));
		if(etimer_expired(&periodic_timer))
		{
			if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) 
			{
				//Send to DAG root
				LOG_INFO("Sending Location Info X,Y => %d, %d\nTo ", loc_x, loc_y);
				LOG_INFO_6ADDR(&dest_ipaddr);
				LOG_INFO_("\n");
				ConvertToMsg(buf, loc_x, loc_y, Location_Info_From_Client, bc_time);
				simple_udp_sendto(&udp_connCBR, &buf, sizeof(buf), &dest_ipaddr);
			} 
			else 
			{
				LOG_INFO("Not reachable yet\n");
			}
			etimer_set(&periodic_timer, SEND_LOCATION_INFO_TO_SERVER_INTERVAL);
		}

		if(etimer_expired(&mass_spring_timer))
		{
			LOG_INFO("Sending Mass Spring Request Info To Nearby People\n");
			uip_create_linklocal_allnodes_mcast(&addr);
			ConvertToMsg(buf, loc_x, loc_y, MASS_SPRING_REQUEST, bc_time);
			simple_udp_sendto(&udp_connCC, &buf, sizeof(buf), &addr);
			etimer_set(&mass_spring_timer, SEND_MASS_SPRING_REQUEST_INFO_INTERVAL);
		}
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
