
#include <avr/io.h>
#include <math.h>
#include "nuts_bolts.h"
#include "settings.h"
#include "eeprom.h"
#include "print.h"
#include "planner.h"
#include <avr/pgmspace.h>
#include "protocol.h"
#include "config.h"
#include "stepper.h"
#include "planner.h"
#include "serial.h"
#include "motion_control.h"

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif 

uint8_t probe_get_status(){
	return 	(PROBE_PIN & PROBE_MASK)?0:1;

}

void probe_init(){
  // set as input
  //PROBE_DDR &= ~(PROBE_MASK);
  cbi(PROBE_DDR, PROBE_PORT_PIN);
  
  // enable pullup
  //PROBE_PORT &= (PROBE_MASK);
  sbi(PROBE_PORT, PROBE_PORT_PIN);
}

uint8_t probe_step_callback(block_t* pBlock){
	
	if((PROBE_PIN & PROBE_MASK) != 0){
		return 0;
	}
	
	
	printPgmString(PSTR("probe tripped \n"));
	plan_set_current_position_n(pBlock->position[X_AXIS], pBlock->position[Y_AXIS], pBlock->position[Z_AXIS]);
	return 1;
}

uint8_t probe_seek(double x, double y, double z, double feed_rate){

  // ensure no other block is pending
  st_synchronize();

  st_set_step_callback(&probe_step_callback);
  mc_line(x, y, z, feed_rate, false);

  // wait for our block to complete
  st_synchronize();
  st_set_step_callback(NULL);

  // check here to see if the current position is less then target or probe is tripped
  // back to where we came from a bit
  // move again at quarter feed rate
  

  return(STATUS_OK);
}