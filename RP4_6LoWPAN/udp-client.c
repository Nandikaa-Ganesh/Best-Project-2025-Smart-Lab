#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "net/packetbuf.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "sys/log.h"
#define LOG_MODULE "Client"
#define LOG_LEVEL LOG_LEVEL_INFO

// To log time related metrics
#include "clock.h"
static clock_time_t boot_time, join_time;
static clock_time_t send_time, recv_time;
static bool joined = false;

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678
#define SEND_INTERVAL (10 * CLOCK_SECOND)

static struct simple_udp_connection udp_conn;
static uint32_t tx_count = 0;
static uint32_t rx_count = 0;
static uint32_t missed_tx_count = 0;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static void udp_rx_callback(struct simple_udp_connection *c,
                            const uip_ipaddr_t *sender_addr,
                            uint16_t sender_port,
                            const uip_ipaddr_t *receiver_addr,
                            uint16_t receiver_port,
                            const uint8_t *data,
                            uint16_t datalen)
{
  recv_time = clock_time();
  LOG_INFO("Received response: '%.*s' from ", datalen, (char *)data);
  LOG_INFO_6ADDR(sender_addr);

  int rssi = (signed short)packetbuf_attr(PACKETBUF_ATTR_RSSI);
  int lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
  LOG_INFO_("\n[Metrics] RSSI: %d dBm, LQI: %d\n", rssi, lqi);

  rx_count++;

  uint32_t latency = (recv_time - send_time) * 1000UL / CLOCK_SECOND;
  LOG_INFO("Round-trip latency: %"PRIu32" ms\n", latency);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static char send_str[] = "send";
  uip_ipaddr_t dest_ipaddr;

  boot_time = clock_time();

  PROCESS_BEGIN();

  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() &&
       NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {

      if(!joined) {
        join_time = clock_time();
        uint32_t elapsed = (join_time - boot_time) * 1000UL / CLOCK_SECOND;
        LOG_INFO("Time to join DAG: %"PRIu32" ms\n", elapsed);
        joined = true;
      }
        
      LOG_INFO("Tx/Rx/Missed: %"PRIu32"/%"PRIu32"/%"PRIu32"\n",
               tx_count, rx_count, missed_tx_count);
      if(tx_count > 0) {
          float failure_ratio = (100.0f * missed_tx_count) / tx_count;
          LOG_INFO("Failure ratio: %.1f%%\n", failure_ratio);
      }
              
      LOG_INFO("Sending periodic request %"PRIu32" to ", tx_count);
      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");

      send_time = clock_time();
      simple_udp_sendto(&udp_conn, send_str, strlen(send_str), &dest_ipaddr);
      tx_count++;
    } else {
      LOG_INFO("Not reachable yet\n");
      if(tx_count > 0) missed_tx_count++;
    }

    etimer_set(&periodic_timer, SEND_INTERVAL
               - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
  }

  PROCESS_END();
}
