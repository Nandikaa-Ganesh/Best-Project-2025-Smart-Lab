#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"

#include "sys/log.h"
#include <stdio.h>

#define LOG_MODULE "Server"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

static struct simple_udp_connection udp_conn;

PROCESS(udp_server_process, "UDP server");
AUTOSTART_PROCESSES(&udp_server_process);

/*---------------------------------------------------------------------------*/
static void udp_rx_callback(struct simple_udp_connection *c,
                            const uip_ipaddr_t *sender_addr,
                            uint16_t sender_port,
                            const uip_ipaddr_t *receiver_addr,
                            uint16_t receiver_port,
                            const uint8_t *data,
                            uint16_t datalen)
{
  LOG_INFO("Received '%.*s' from ", datalen, (char *)data);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");

  if(strncmp((char *)data, "send", datalen) == 0) {
    FILE *fp = fopen("/home/mukeshbala/Desktop/comp_output/encoded_data.txt", "r");
    if(fp == NULL) {
      char msg[] = "File not found";
      simple_udp_sendto(&udp_conn, msg, strlen(msg), sender_addr);
      return;
    }

    char line[600], last_line[600];
    memset(last_line, 0, sizeof(last_line));
    while(fgets(line, sizeof(line), fp) != NULL) {
      strcpy(last_line, line);
    }
    fclose(fp);

    if(strlen(last_line) > 0) {
      simple_udp_sendto(&udp_conn, last_line, strlen(last_line), sender_addr);
      LOG_INFO("Sent last line to client\n");
    } else {
      char msg[] = "File is empty";
      simple_udp_sendto(&udp_conn, msg, strlen(msg), sender_addr);
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  PROCESS_BEGIN();

  //NETSTACK_ROUTING.root_start();

  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);

  PROCESS_END();
}
