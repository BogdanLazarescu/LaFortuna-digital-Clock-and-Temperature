
#ifndef __TFP_PRINTF__

#define __TFP_PRINTF__


#include <stdarg.h>
//#include <lcd.h>


void tfp_printf(char *fmt, ...);

#define printf tfp_printf 


#endif

