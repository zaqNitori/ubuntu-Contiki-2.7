/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "random.h"
#include "net/routing/rpl-location/rpl-loc.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  0
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678
#define UDP_BR_PORT 3333

#define SEND_INTERVAL		  (10 * CLOCK_SECOND)
#define SEND_LOCATION_INFO_INTERVAL	(60 * CLOCK_SECOND)

static struct simple_udp_connection udp_connSC;
static struct simple_udp_connection udp_connSBR;

PROCESS(udp_server_process, "UDP server");
AUTOSTART_PROCESSES(&udp_server_process);
/*---------------------------------------------------------------------------*/
static void
udp_rx_SBR_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
	LOG_INFO("Received Msg \nFrom ");
	LOG_INFO_6ADDR(sender_addr);
	LOG_INFO_("\n");

}
/*---------------------------------------------------------------------------*/
static void
udp_rx_SC_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
	LOG_INFO("Received Msg \nFrom ");
	LOG_INFO_6ADDR(sender_addr);
	LOG_INFO_("\n");
	
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{

	uip_ipaddr_t addr;
	uip_ipaddr_t dest_ipaddr;
	static struct etimer periodic_timer;
	static struct etimer location_info_timer;
	char buf[30];
	loc_x = loc_y = 30;
	bc_time = 0;
	

	PROCESS_BEGIN();

	/* Initialize DAG root */
	//NETSTACK_ROUTING.root_start();

	/* Initialize UDP connection */
	simple_udp_register(&udp_connSBR, UDP_SERVER_PORT, NULL,
                      UDP_BR_PORT, udp_rx_SBR_callback);
	simple_udp_register(&udp_connSC, UDP_SERVER_PORT, NULL,
		              UDP_CLIENT_PORT, udp_rx_SC_callback);

	etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
	etimer_set(&location_info_timer, random_rand() % SEND_LOCATION_INFO_INTERVAL);

	while(1)
	{
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer) || etimer_expired(&location_info_timer));
		if(etimer_expired(&location_info_timer))
		{
			if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) 
			{
				//Send to DAG root
				LOG_INFO("Sending Location Info X,Y => %d, %d\nTo BR => ", loc_x, loc_y);
				LOG_INFO_6ADDR(&dest_ipaddr);
				LOG_INFO_("\n");
				ConvertToMsg(buf, loc_x, loc_y, Location_Info_From_Server, bc_time);
				simple_udp_sendto(&udp_connSBR, &buf, sizeof(buf), &dest_ipaddr);
			} 
			else 
			{
				LOG_INFO("Not reachable yet\n");
			}
			etimer_set(&location_info_timer, random_rand() % SEND_LOCATION_INFO_INTERVAL);
		}

		if(etimer_expired(&periodic_timer))
		{
			LOG_INFO("Sending Location Information\n");

			ConvertToMsg(buf, loc_x * 1000, loc_y * 1000, Location_Info, bc_time);
			uip_create_linklocal_allnodes_mcast(&addr);
			simple_udp_sendto(&udp_connSC, &buf, sizeof(buf), &addr);

			etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL); 
		}
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/



