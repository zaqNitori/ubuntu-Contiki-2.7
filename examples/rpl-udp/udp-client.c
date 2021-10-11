#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "net/routing/rpl-location/rpl-loc.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SEND_MASS_SPRING_REQUEST_INFO_INTERVAL (20 * CLOCK_SECOND)
#define SEND_LOCATION_INFO_TO_SERVER_INTERVAL (30 * CLOCK_SECOND)

static struct simple_udp_connection udp_connCS;
static struct simple_udp_connection udp_connCC;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static int mass_spring_model_localization()
{
	return 1;
}
/*---------------------------------------------------------------------------*/
static int 
Do_Mass_Spring_Model_Localization(int x, int y)
{
	radio_value_t v;
	NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_RSSI,&v);
	printf("Received RSSI => %d\nFrom ",v);
	return mass_spring_model_localization();
}
/*---------------------------------------------------------------------------*/
static void 
Pass_On_Location_Information(rpl_loc_msg_t msg)
{
	uip_ipaddr_t addr;

	uip_create_linklocal_allnodes_mcast(&addr);
	simple_udp_sendto(&udp_connCC, &msg, sizeof(msg), &addr);	

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
	rpl_loc_msg_t msg = *(rpl_loc_msg_t *)data;

	if (msg.Msg_Type == Location_Info)
	{
		if (msg.bc_time < bc_time)
		{
			bc_time = ++msg.bc_time;
			//msg.x = loc_x;
			//msg.y = loc_y;			
			
			Do_Mass_Spring_Model_Localization(msg.x, msg.y);
			LOG_INFO_6ADDR(sender_addr);
			LOG_INFO_("\n");

			printf("My bc_time is %d\n", bc_time);
			printf("My x, y is %d, %d\n", loc_x, loc_y);

			Pass_On_Location_Information(msg);
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
	rpl_loc_msg_t msg = *(rpl_loc_msg_t *)data;

	if (msg.Msg_Type == Location_Info)
	{
		if (msg.bc_time < bc_time)
		{
			bc_time = ++msg.bc_time;
			//msg.x = loc_x;
			//msg.y = loc_y;			

			Do_Mass_Spring_Model_Localization(msg.x, msg.y);
			LOG_INFO_6ADDR(sender_addr);
			LOG_INFO_("\n");

			printf("My bc_time is %d\n", bc_time);
			printf("My x, y is %d, %d\n", loc_x, loc_y);

			Pass_On_Location_Information(msg);
		}
	}

}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
	static struct etimer periodic_timer;
	uip_ipaddr_t dest_ipaddr;
	rpl_loc_msg_t msg;
	if(!start)
	{
		bc_time = 100;
		loc_x = random_rand() % 101;
		loc_y = random_rand() % 101;
		start = 1;
	}

	PROCESS_BEGIN();

	/* Initialize UDP connection */
	simple_udp_register(&udp_connCS, UDP_CLIENT_PORT, NULL,
		              UDP_SERVER_PORT, udp_rx_CS_callback);
	simple_udp_register(&udp_connCC, UDP_CLIENT_PORT, NULL,
		              UDP_CLIENT_PORT, udp_rx_CC_callback);


	etimer_set(&periodic_timer, SEND_LOCATION_INFO_TO_SERVER_INTERVAL);
	while(1) {
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

	if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) 
	{
		/* Send to DAG root */
		LOG_INFO("Sending Location Info X,Y => %d, %d\nTo ", loc_x, loc_y);
		LOG_INFO_6ADDR(&dest_ipaddr);
		LOG_INFO_("\n");
		msg.x = loc_x;
		msg.y = loc_y;
		msg.Msg_Type = Location_Info_From_Client;
		simple_udp_sendto(&udp_connCS, &msg, sizeof(msg), &dest_ipaddr);
	} 
	else 
	{
		LOG_INFO("Not reachable yet\n");
	}

		/* Add some jitter */
		//etimer_set(&periodic_timer, SEND_LOCATION_INFO_TO_SERVER_INTERVAL
	  	//- CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
		etimer_set(&periodic_timer, SEND_LOCATION_INFO_TO_SERVER_INTERVAL);
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
