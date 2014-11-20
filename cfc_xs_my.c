// SpiNNaker API
#include "spin1_api.h"

// ------------------------------------------------------------------------
// constants
// ------------------------------------------------------------------------
#define TIMER_TICK_PERIOD  1000       // (in useconds)
#define RUN_TIME           10         // (in seconds)
#define TIMEOUT            (RUN_TIME * 1000000 / TIMER_TICK_PERIOD)

//#define LIMIT          473000  // ok
//#define LIMIT             473  // ok
//#define LIMIT             100  // ok
//#define LIMIT              72  // ok
//#define LIMIT              69
//#define LIMIT              68  // ok (limit for 4 cores)
//#define LIMIT              67
//#define LIMIT              55  // ok (new limit for 4 cores)
//#define LIMIT              53  // ok (limit for 3 cores)
//#define LIMIT              51  // ok
//#define LIMIT              46  // ok
//#define LIMIT              40  // ok (new limit for 3 cores)
//#define LIMIT              39  // ok
//#define LIMIT              37  // ok
//#define LIMIT              33  // ok (limit for 2 cores)
//#define LIMIT              31  // ok
//#define LIMIT              30  // ok
//#define LIMIT              26  // ok (new limit for 2 cores)
//#define LIMIT              25  // ok (limit for 6 cores native)
//#define LIMIT               22  // ok (limit for 1 core)
//#define LIMIT               21  // lossy (limit for 4 cores native)
//#define LIMIT               3  // lossy
#define LIMIT               0  // lossy

  // limit settings:
  // 0.05 473
  // 0.10 233
  // 0.20 112
  // 0.30  73
  // 0.35  62
  // 0.40  53
  // 0.45  46
  // 0.50  41
  // 0.55  37
  // 0.60  33
  // 0.70  28
  // 0.80  23


// ------------------------------------------------------------------------
// macros
// ------------------------------------------------------------------------
#define CHIP(x, y)         ((x << 8) | y)
#define ROUTE_TO_CORE(c)   (1 << (c + 6))
#define ROUTE_TO_LINK(l)   (1 << l)

#define TST_MASK           0xff000000


// ------------------------------------------------------------------------
// variables
// ------------------------------------------------------------------------
uint coreID;
uint chipID;

uint fpga_key;
uint core_key;
uint key_cnt = 0;

uint entry;

uint packets = 0;

volatile uchar stop = 0;

uint app_init ();
void send_pkts (uint a, uint b);
void timeout (uint ticks, uint null);

void c_main()
{
  uint res;

  io_printf (IO_STD, ">> ls\n");

  /* get this core's IDs */
  coreID = spin1_get_core_id ();
  chipID = spin1_get_chip_id ();

  /* turn emergency routing on! */
//#  rtr[RTR_CONTROL] |= 0x0a000000;

  /* set timer tick value (in microseconds) */
  spin1_set_timer_tick (TIMER_TICK_PERIOD);

  /* register callbacks */
  spin1_callback_on (TIMER_TICK, timeout, -1);

  /* initialize application */
  res = app_init ();

  if (res == SUCCESS)
  {
    /* go */
//#!    spin1_start (SYNC_NOWAIT);
    spin1_start (SYNC_WAIT);

    // invalidate routing entry
//#    rtr_free (entry, 1);

    /* report results */
    io_printf (IO_STD, "sent %d packets\n", packets);
  }

  io_printf (IO_STD, "<< ls\n");
}


uint app_init ()
{
  uint result = FAILURE;

  core_key = coreID << 24;
  fpga_key = (0x80000000 | core_key);

  // turn on packet error counter
  rtr[RTR_CONTROL] |= 0x00000030;

  // initialise MC routing table entries
  // -------------------------------------------------------------------
  uint fpga_rt;

  result = SUCCESS;
  if (chipID==CHIP(0,0) || chipID==CHIP(0,1) || chipID==CHIP(0,2) || chipID==CHIP(0,3))
  {
    if (coreID==3 || coreID==5 || coreID==6 || coreID==8)
      fpga_rt = ROUTE_TO_LINK(3);
    else if (coreID==2 || coreID==4 || coreID==7 || coreID==9)
      fpga_rt = ROUTE_TO_LINK(4);
    else
    {
      fpga_rt = 0;  // throw away!
      result = FAILURE;
    }
  }
  else if (chipID==CHIP(11,0) || chipID==CHIP(11,1) || chipID==CHIP(11,2) || chipID==CHIP(11,3))
  {
    if (coreID==11 || coreID==13 || coreID==14 || coreID==16)
      fpga_rt = ROUTE_TO_LINK (0);
    else
    {
      fpga_rt = 0;  // throw away!
      result = FAILURE;
    }
  }

  if (result == SUCCESS)
  {
    entry = rtr_alloc (1);

    if (entry == 0)
    {
      result = FAILURE;
    }
    else
    {
      // send packets with fpga_key to FPGA
      rtr_mc_set (entry,      // entry
                  fpga_key,   // key
                  TST_MASK,   // mask
                  fpga_rt     // route
                 );
    }
  }

  return (result);
}

void timeout (uint ticks, uint null)
{

  if (ticks == 1)
  {
    // kick-start the sending of packets
    spin1_schedule_callback (send_pkts, 0, 0, 3);
  }
  else if (ticks == (TIMEOUT + 1))
  {
    // stop sending packets
    stop = 1;
  }
  else if (ticks > (TIMEOUT + 10)) // make sure to receive all packets!
  {
    // finish simulation
    spin1_exit (0);
  }
}

void send_pkts (uint a, uint b)
{
  // start fpga!
/* //# cc[CC_TXKEY] = fpga_key | 0xffff;
  packets++;
  for (uint i = 0; i < LIMIT; i++);*/

  while (!stop)
  {
    while (!(cc[CC_TCR] & 0x10000000));
//#    cc[CC_TXDATA] = key_cnt << 4;
    cc[CC_TXKEY] = fpga_key | key_cnt;
    key_cnt++;
//#    if (key_cnt == 0xfffe) key_cnt = 0;
    packets++;

    for (uint i = 0; i < LIMIT; i++);
  }
}


