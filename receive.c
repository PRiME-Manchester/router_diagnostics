// SpiNNaker API
#include "spin1_api.h"

// ------------------------------------------------------------------------
// constants
// ------------------------------------------------------------------------
#define TIMER_TICK_PERIOD  1000       // (in useconds)
#define RUN_TIME           10         // (in seconds)
#define TIMEOUT            (RUN_TIME * 1000000 / TIMER_TICK_PERIOD)
