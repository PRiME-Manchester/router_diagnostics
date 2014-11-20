// SpiNNaker API
#include "spin1_api.h"
//#include "spinn_io.h"

// ------------------------------------------------------------------------
// constants
// ------------------------------------------------------------------------
#define TIMER_TICK_PERIOD  100000   // 10th of Second
#define TIMEOUT            300      // ticks (30 seconds)
//#define TIMEOUT          1        // ticks (10th of a second)
#define PKT_CNT            10000    // to indicate life
#define BUFFER_SIZE        0x100000 // key buffer (in SDRAM)


#define LIMIT              99  // throttle packet reception


// ------------------------------------------------------------------------
// variables
// ------------------------------------------------------------------------
uint coreID;
uint chipID;
uint packets = 0;
uint wpkts   = 0;
uint key_err = 0;
uint key_tot = 0;
uint key_pre;

uint entry;

uint lst_key = 0;
uint lst_pld = 0;

uint *sdram_buffer;
uint sdram_addr = 1;

uint app_init ();
void count_packets (uint key, uint payload);
void timertick (uint ticks, uint null);

// ------------------------------------------------------------------------
// main
// ------------------------------------------------------------------------
void c_main()
{
  io_printf (IO_STD, ">> pr\n");

  /* get this core's IDs */
  coreID = spin1_get_core_id ();
  chipID = spin1_get_chip_id ();

  /* set timer tick value (in microseconds) */
  spin1_set_timer_tick (TIMER_TICK_PERIOD);

  /* register callbacks */
  spin1_callback_on (MC_PACKET_RECEIVED, count_packets, 0);
  spin1_callback_on (MCPL_PACKET_RECEIVED, count_packets, 0);
  spin1_callback_on (TIMER_TICK, timertick, -1);

  /* initialize application */
  uint res = app_init ();

  if (res == SUCCESS)
  {
    // go
    spin1_start (SYNC_NOWAIT);

    // invalidate routing entry
//#    rtr_free (entry, 1);

    // report results
    spin1_delay_us (1000 + 250 * (chipID << 5) + 200 * (coreID << 1));
    io_printf (IO_STD, "last packet received 0x%08x:%08x\n",
                lst_key, lst_pld
              );
    io_printf (IO_STD, "received %d packets\n", packets);
    io_printf (IO_STD, "%d sequence errors\n", key_err);
    io_printf (IO_STD, "cum. diff: %d\n", key_tot);

    if (wpkts != 0)
    {
      spin1_delay_us(1000);
      io_printf (IO_STD, "wpkts: %d\n", wpkts);
    }

    io_printf (IO_STD, "<< pr\n");
  }
}

// ------------------------------------------------------------------------
// functions
// ------------------------------------------------------------------------
uint app_init ()
{
  entry = rtr_alloc (1);

  if (entry != 0)
  {
    // initialise pointer to key buffer in SDRAM
    sdram_buffer  = (uint *) (SPINN_SDRAM_BASE +
                               ((coreID - 1) * BUFFER_SIZE * sizeof(uint))
                             );

    // turn on packet error counter
    rtr[RTR_CONTROL] |= 0x00000030;

    // initialise MC routing table entries
    // -------------------------------------------------------------------
    // set a MC routing table entry to send all packets back to me
    rtr_mc_set (entry,                      // entry
                 coreID << 24,              // key
                 0x7f000000,                // mask
                 (uint) (1 << (coreID+6))   // route
               );

    return (SUCCESS);
  }
  else
  {
    return (FAILURE);
  }
}


void count_packets (uint key, uint payload)
{
  // remember key and payload
  lst_key = key;
  lst_pld = payload;

  // store key
//#  if (sdram_addr < BUFFER_SIZE)
//#    sdram_buffer[sdram_addr++] = key;

  // check for key sequence errors
  if ((packets != 0) && (key != (key_pre + 1)))
  {
    key_err++;
    key_tot += (key - key_pre - 1);
  }
  key_pre = key;

  // count packets
  if ((++packets % PKT_CNT) == 0)
  {

//####    for (uint i = 0; i < 33; i++);

    // flip led 1
    //spin1_led_control(LED_INV (1));

    // print packet data
//#    io_printf (IO_STD, "k: 0x%08x -- pr: %d\n", key, packets);
    //io_printf (IO_STD, "pr: %d\n", packets);
    //io_printf (IO_STD, "key: %u - payload: %u\n", key, payload);
  }

  // check for special keys
/*  if ((key & 0xff000000) == 0xbc000000)
  {
//#    io_printf (IO_STD, "wk: 0x%08x -- pr: %d\n", key, packets);
    wpkts++;
    }*/

  // throttle packet reception
//#  if ((coreID == 3) || (coreID == 6))
//#    for (uint i = 0; i < LIMIT; i++);
}


void timertick (uint ticks, uint null)
{
  if (ticks >= TIMEOUT)
  {
    // store last address
    sdram_buffer[0] = sdram_addr - 1;

    // finish simulation
    spin1_exit (0);
  }
}



