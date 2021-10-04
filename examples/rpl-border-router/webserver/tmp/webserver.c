/*
 * Copyright (c) 2017, RISE SICS
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
#include "sys/log.h"
#include "net/routing/routing.h"
#include "net/ipv6/uip-ds6-nbr.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/ipv6/uip-sr.h"
#include "net/ipv6/simple-udp.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "httpd-simple.h"
#include "net/routing/rpl-location/rpl-loc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LOG_MODULE "RPL WEB"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  0
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678
#define SEND_INTERVAL		  (30 * CLOCK_SECOND)
static struct simple_udp_connection udp_connSC;
static struct simple_udp_connection udp_connSS;
PROCESS(web_sense_process, "Sense Web Demo");
int x;
int y;
int w;
char result[30];
char temp[30];
int count = 0;
const uip_ipaddr_t *seat;
/*---------------------------------------------------------------------------*/
static char buf[256];
static int blen;
#define ADD(...) do {                                                   \
    blen += snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__);      \
  } while(0)
#define SEND(s) do { \
  SEND_STRING(s, buf); \
  blen = 0; \
} while(0);

/* Use simple webserver with only one page for minimum footprint.
 * Multiple connections can result in interleaved tcp segments since
 * a single static buffer is used for all segments.
 */
#include "httpd-simple.h"

/*-------------------------------------------------------------*/
struct node_t{
    char address[30];
    int x;
    int y;
};

struct node_t nodes[3];


/*-----------------------------------------------------------*/
void call(){
	for(w=0 ; w<3 ; w++){
	    if(w==0)
		strcpy(nodes[w].address,"fe80::204:4:4:4");
	    else if(w==1)
		strcpy(nodes[w].address,"fe80::205:5:5:5");
	    else if(w==2)
		strcpy(nodes[w].address,"fe80::206:6:6:6");
	    //nodes[w].x=0;
	    //nodes[w].y=0;

	}
}
/*-------------------------------------------------------------------*/
void change(char a){
    if(a=='0'){
    	ADD("0");
    }
    else if(a=='1'){
    	ADD("1");
    }
    else if(a=='2'){
    	ADD("2");
    }
    else if(a=='3'){
    	ADD("3");
    }
    else if(a=='4'){
    	ADD("4");
    }
    else if(a=='5'){
    	ADD("5");
    }
    else if(a=='6'){
    	ADD("6");
    }
    else if(a=='7'){
    	ADD("7");
    }
    else if(a=='8'){
    	ADD("8");
    }
    else if(a=='9'){
    	ADD("9");
    }
    else if(a=='a'){
    	ADD("a");
    }
    else if(a=='b'){
    	ADD("b");
    }
    else if(a=='c'){
    	ADD("c");
    }
    else if(a=='d'){
    	ADD("d");
    }
    else if(a=='e'){
    	ADD("e");
    }
    else if(a=='f'){
    	ADD("f");
    }
    else if(a==':'){
    	ADD(":");
    }

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
  rpl_loc_msg_t msg;
	msg = *(rpl_loc_msg_t *)data;
	//printf("get msg from ");
  seat = sender_addr;
  printf("\nWeb\n");
  LOG_INFO_6ADDR(sender_addr);
  printf("\n");
  LOG_INFO_6ADDR(seat);
  if(msg.msgType == msg4)
	{
    printf("\nx = %d, y = %d\n", msg.loc_x, msg.loc_y);
    x = msg.loc_x;
    y = msg.loc_y;
    printf("\nWeb2\n");
    uint16_t a;
  int i;
  int j,k;
    
    for(i=0;i<30;i++){
	result[i] = '\n';
	temp[i] = '\n';
        count=0;
    }
    	
    for(i = 0; i < sizeof(uip_ipaddr_t)/2; i ++) {
     
    a = (seat->u16[i]);
    sprintf(temp, "%x", a); 
    if(i==0){
	result[count++] = temp[2];
	result[count++] = temp[3];
	result[count++] = temp[0];
	result[count++] = temp[1];
    }
    else if(i==1){
	result[count++] = ':';

    }
    else if(i==2){
	result[count++] = ':';
    }
    else if(i==3){
       
    }
    else if(i==4){

	result[count++] = temp[2];
	result[count++] = temp[1];
	result[count++] = temp[0];
        
    }
    else if(i==5){
	result[count++] = ':';
        //ADD(":");
	//printf("\n:");
	for(j=2;j>=0;j--){
	   if(temp[j]!='0')
		break;	
	}
	for(k=j;k>=0;k--){
	     result[count++] = temp[k]; 	
	}
    }
    else if(i==6){
	result[count++] = ':';
	//ADD(":");
	//printf("\n:");
	for(j=2;j>=0;j--){
	   if(temp[j]!='0')
		break;	
	}
	for(k=j;k>=0;k--){
	//     printf("\n%c",temp[k]);
	  // ADD("%c",temp[k]);	
	    result[count++] = temp[k]; 
	}
    }
    else if(i==7){
	//ADD(":");
	result[count++] = ':';
	//printf("\n:");
	for(j=2;j>=0;j--){
	   if(temp[j]!='0')
	       break;	
	}
	for(k=j;k>=0;k--){
	//     printf("\n%c",temp[k]);
	       result[count++] = temp[k]; 
	}
    }
    }
    for(w=0;w<3;w++){
	if((strcmp(nodes[w].address,result))==-10){
            printf("\nHERE\n");
            printf("%s\n",nodes[w].address);
	    nodes[w].x = msg.loc_x;
	    nodes[w].y = msg.loc_y;
	}
    }
    }
    //simple_udp_sendto(&udp_connSC, &msg, sizeof(msg), &addr);
  

}

/*-------------------------------------------------------------------------*/
static void
udp_rx_SS_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  
}

/*---------------------------------------------------------------------------*/
static
PT_THREAD(generate_routes(struct httpd_state *s))
{
  //int number = 0;
  int i;
  PSOCK_BEGIN(&s->sout);
  
  
  printf("send json to requester\n");
  ADD("{\"nodes\":[");
  int z;
  for(i=0;i<3;i++){
      if(i!=0)
	ADD(",");
      ADD("{\"names\":\"");
      for(z=0;z<strlen(nodes[i].address);z++)
      	change(nodes[i].address[z]);
      ADD("\",\"X\":%d,\"Y\":%d}",nodes[i].x,nodes[i].y);
  }
  ADD("]}");
  SEND(&s->sout);
  PSOCK_END(&s->sout);
}

/*---------------------------------------------------------------------------*/
PROCESS(webserver_nogui_process, "Web server");
PROCESS_THREAD(webserver_nogui_process, ev, data)
{
  PROCESS_BEGIN();
  
  httpd_init();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    httpd_appcall(data);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
httpd_simple_script_t
httpd_simple_get_script(const char *name)
{
  return generate_routes;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(web_sense_process, ev, data)
{
  static struct etimer periodic_timer;
  uip_ipaddr_t addr;
  static rpl_loc_msg_t msg;
  call();
  PROCESS_BEGIN();

  

  etimer_set(&periodic_timer, SEND_INTERVAL);
  simple_udp_register(&udp_connSC, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_SC_callback);
  simple_udp_register(&udp_connSS, UDP_SERVER_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_SS_callback);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    etimer_reset(&periodic_timer);

    uip_create_linklocal_allnodes_mcast(&addr);
	  simple_udp_sendto(&udp_connSC, &msg, sizeof(msg), &addr);
  }

  PROCESS_END();
}
