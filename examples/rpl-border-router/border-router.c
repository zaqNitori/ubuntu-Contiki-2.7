/*
 * Copyright (c) 201, RISE SICS
 * All rights reserved.
 *
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

/* Log configuration */
#include "sys/log.h"
#include "net/ipv6/simple-udp.h"
#include "net/netstack.h"
#define LOG_MODULE "RPL BR"
#define LOG_LEVEL LOG_LEVEL_INFO
#include "net/routing/rpl-location/rpl-loc.h"

#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678
#define UDP_BR_PORT 3333
static struct simple_udp_connection udp_connBRC;
static struct simple_udp_connection udp_connBRS;
#define SEND_INTERVAL		  (30 * CLOCK_SECOND)


/* Declare and auto-start this file's process */
PROCESS(contiki_ng_br, "Contiki-NG Border Router");
AUTOSTART_PROCESSES(&contiki_ng_br);
/*---------------------------------------------------------------------------*/
static void
udp_rx_BRS_callback(struct simple_udp_connection *c,
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
udp_rx_BRC_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{

	int msg_x, msg_y, msg_type, msg_bc;
	GetMsg((char*)data, &msg_x, &msg_y, &msg_type, &msg_bc);
	if(msg_type == Location_Info_From_Client)
	{
		LOG_INFO("Received Location Info X,Y => %d, %d\nFrom ", msg_x, msg_y);
		LOG_INFO_6ADDR(sender_addr);
		LOG_INFO_("\n");
	}

}


/*---------------------------------------------------------------------------*/
PROCESS_THREAD(contiki_ng_br, ev, data)
{
	static struct etimer periodic_timer;
	uip_ipaddr_t addr;

	PROCESS_BEGIN();


	#if BORDER_ROUTER_CONF_WEBSERVER
		PROCESS_NAME(webserver_nogui_process);
		process_start(&webserver_nogui_process, NULL);
	#endif /* BORDER_ROUTER_CONF_WEBSERVER */

	simple_udp_register(&udp_connBRS, UDP_BR_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_BRS_callback);
	simple_udp_register(&udp_connBRC, UDP_BR_PORT, NULL,
		              UDP_CLIENT_PORT, udp_rx_BRC_callback);

	LOG_INFO("Contiki-NG Border Router started\n");

	etimer_set(&periodic_timer, SEND_INTERVAL);

	while(1)
	{
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
		uip_create_linklocal_allnodes_mcast(&addr);
		etimer_set(&periodic_timer, SEND_INTERVAL); 
	}

	PROCESS_END();
}
