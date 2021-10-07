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

#define SEND_INTERVAL		  (10 * CLOCK_SECOND)

static struct simple_udp_connection udp_conn;
static struct simple_udp_connection udp_connSS;
uip_ipaddr_t BRaddr;
static int ok = 0;

PROCESS(udp_server_process, "UDP server");
AUTOSTART_PROCESSES(&udp_server_process);

/*---------------------------------------------------------------------------*/
static void
udp_rx_SS_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{

  BRaddr = *sender_addr;
ok = 1;

}

/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{

int msg = *(int *)data;
  LOG_INFO("Received msg => %d from ", msg);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");
if(ok)
{
rpl_loc_msg_t msg2;
msg2.x = msg;
msg2.addr = *sender_addr;
simple_udp_sendto(&udp_connSS, &msg2, sizeof(msg2), &BRaddr);
}

#if WITH_SERVER_REPLY
  /* send back the same string to the client as an echo reply */
  LOG_INFO("Sending response.\n");
  simple_udp_sendto(&udp_conn, data, datalen, sender_addr);
#endif /* WITH_SERVER_REPLY */
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{

	uip_ipaddr_t addr;
	static struct etimer periodic_timer;
	static char str[32];
	static unsigned count;
	//static int num = 1;

  PROCESS_BEGIN();

  /* Initialize DAG root */
  //NETSTACK_ROUTING.root_start();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);
simple_udp_register(&udp_connSS, UDP_SERVER_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_SS_callback);

	etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);

	while(1)
	{
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
		LOG_INFO("Sending message");
	      LOG_INFO_("\n");
	      snprintf(str, sizeof(str), "Server msg %d", count);
		uip_create_linklocal_allnodes_mcast(&addr);
    		//simple_udp_sendto(&udp_conn, &num, sizeof(num), &addr);
		count++;

		etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL); 
	}

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/



