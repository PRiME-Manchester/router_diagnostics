// API
#include "spin1_api.h"

// CONSTANTS
#define TIMER_TICK_PERIOD  1000       // (in us)
#define RUN_TIME           10         // (in seconds)
#define TIMEOUT            (RUN_TIME*1000000/TIMER_TICK_PERIOD)
#define LIMIT              10
#define MASK_ALL           0xffffffff

// MACROS
#define CHIP(x, y)         ((x << 8) | y)
#define TST_MASK           0xff000000

// VARIABLES
uint coreID;
uint chipID;
uint core_key;
uint e;
uint packets = 0;
uint key_cnt = 0;

volatile uchar stop = 0;

void app_init(void);
void send_pkts(uint a, uint b);
void timeout(uint ticks, uint null);
void count_packets();

// MAIN
void c_main()
{
  io_printf (IO_STD, ">> ls\n");

  // Get this core's IDs
  coreID = spin1_get_core_id();
  chipID = spin1_get_chip_id();

  // Turn emergency routing on!
//#  rtr[RTR_CONTROL] |= 0x0a000000;

  // Set timer tick value (in us)
  spin1_set_timer_tick(TIMER_TICK_PERIOD);

  // Register callbacks
  spin1_callback_on(MCPL_PACKET_RECEIVED, count_packets, -1);
  spin1_callback_on(TIMER_TICK, timeout, 1);

  // Initialize application
  app_init();
  
  // Go
  spin1_start (SYNC_WAIT);

  // Invalidate routing entry
//#    rtr_free (entry, 1);

  // Report results
  io_printf (IO_STD, "sent %d packets\n", packets);

  io_printf (IO_STD, "<< ls\n");
}

// Router setup
void app_init(void)
{
  core_key = coreID << 24;
  // fpga_key = (0x80000000 | core_key);

  // Turn on packet error counter
  rtr[RTR_CONTROL] |= 0x00000030;

  // Initialise MC routing table entries
  if (coreID==1)
  {
    e = rtr_alloc(1);
    if (!e)
      rt_error(RTE_ABORT);
  }
  
  // All chips (Cores 1-4 -> 5)
  if (coreID==1 || coreID==2 || coreID==3 || coreID==4)
    rtr_mc_set(e, 1, MASK_ALL, MC_CORE_ROUTE(5));
}

void timeout(uint ticks, uint null)
{

  if (ticks == 1)
  {
    // Kick-start the sending of packets
    spin1_schedule_callback (send_pkts, 0, 0, 3);
  }
  else if (ticks == (TIMEOUT + 1))
  {
    // Stop sending packets
    stop = 1;
  }
  else if (ticks > (TIMEOUT + 10)) // make sure to receive all packets!
  {
    // Finish simulation
    spin1_exit (0);
  }
}

void send_pkts(uint a, uint b)
{
  while (!stop)
  {
    while (!(cc[CC_TCR] & 0x10000000));
//#    cc[CC_TXDATA] = key_cnt << 4;
//#    cc[CC_TXKEY] = fpga_key | key_cnt;
    key_cnt++;
//#    if (key_cnt == 0xfffe) key_cnt = 0;
    packets++;

    #ifdef LIMIT
      for (uint i=0; i<LIMIT; i++);
    #endif
  }
}

void count_packets(uint a, uint b)
{

}



