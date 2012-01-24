/*
  limits.h - code pertaining to limit-switches and performing the homing cycle
  Part of Grbl

  Copyright (c) 2009-2011 Simen Svale Skogsrud

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/
  
#include <util/delay.h>
#include <avr/io.h>
#include "stepper.h"
#include "settings.h"
#include "nuts_bolts.h"
#include "config.h"
#include "motion_control.h"
#include "planner.h"
#include <avr/pgmspace.h>
#include "print.h"
#include "status.h"

void limits_init() {
	// configure as input
  //LIMIT_DDR &= ~(LIMIT_MASK);
  cbi(LIMIT_DDR, X_LIMIT_BIT);
  cbi(LIMIT_DDR, Y_LIMIT_BIT);
  cbi(LIMIT_DDR, Z_LIMIT_BIT);
}
/*
static void homing_cycle(bool x_axis, bool y_axis, bool z_axis, bool reverse_direction, uint32_t microseconds_per_pulse) {
  // First home the Z axis
  uint32_t step_delay = microseconds_per_pulse - settings.pulse_microseconds;
  uint8_t out_bits = DIRECTION_MASK;
  uint8_t limit_bits;
  
  if (x_axis) { out_bits |= (1<<X_STEP_BIT); }
  if (y_axis) { out_bits |= (1<<Y_STEP_BIT); }
  if (z_axis) { out_bits |= (1<<Z_STEP_BIT); }
  
  // Invert direction bits if this is a reverse homing_cycle
  if (reverse_direction) {
    out_bits ^= DIRECTION_MASK;
  }
  
  // Apply the global invert mask
  out_bits ^= settings.invert_mask;
  
  // Set direction pins
  STEPPING_PORT = (STEPPING_PORT & ~DIRECTION_MASK) | (out_bits & DIRECTION_MASK);
  
  for(;;) {
    limit_bits = LIMIT_PIN;
    if (reverse_direction) {         
      // Invert limit_bits if this is a reverse homing_cycle
      limit_bits ^= LIMIT_MASK;
    }
    if (x_axis && !(LIMIT_PIN & (1<<X_LIMIT_BIT))) {
      x_axis = false;
      out_bits ^= (1<<X_STEP_BIT);      
    }    
    if (y_axis && !(LIMIT_PIN & (1<<Y_LIMIT_BIT))) {
      y_axis = false;
      out_bits ^= (1<<Y_STEP_BIT);
    }    
    if (z_axis && !(LIMIT_PIN & (1<<Z_LIMIT_BIT))) {
      z_axis = false;
      out_bits ^= (1<<Z_STEP_BIT);
    }
    // Check if we are done
    if(!(x_axis || y_axis || z_axis)) { return; }
    STEPPING_PORT |= out_bits & STEP_MASK;
    _delay_us(settings.pulse_microseconds);
    STEPPING_PORT ^= out_bits & STEP_MASK;
    _delay_us(step_delay);
  }
  return;
}

static void approach_limit_switch(bool x, bool y, bool z) {
  homing_cycle(x, y, z, false, 100000);
}

static void leave_limit_switch(bool x, bool y, bool z) {
  homing_cycle(x, y, z, true, 500000);
}

void limits_go_home() {
  st_synchronize();
  // Store the current limit switch state
  uint8_t original_limit_state = LIMIT_PIN;
  approach_limit_switch(false, false, true); // First home the z axis
  approach_limit_switch(true, true, false);  // Then home the x and y axis
  // Xor previous and current limit switch state to determine which were high then but have become 
  // low now. These are the actual installed limit switches.
  uint8_t limit_switches_present = (original_limit_state ^ LIMIT_PIN) & LIMIT_MASK;
  // Now carefully leave the limit switches
  leave_limit_switch(
    limit_switches_present & (1<<X_LIMIT_BIT), 
    limit_switches_present & (1<<Y_LIMIT_BIT),
    limit_switches_present & (1<<Z_LIMIT_BIT));
}

*/


static uint8_t flags = 0;
#define LIMIT_REVERSE		(1<<0)
#define LIMIT_X				(1<<1)
#define LIMIT_Y				(1<<2)
#define LIMIT_Z				(1<<3)

uint8_t limit_step_callback(block_t* pBlock){
	uint8_t limit_bits = LIMIT_PIN & LIMIT_MASK;
	uint8_t result = STEPPER_INHIBIT_ALL;
	
	if(flags & LIMIT_REVERSE){
      // Invert limit_bits if this is a reverse homing_cycle
      limit_bits ^= LIMIT_MASK;
	}
	
	if(flags & LIMIT_X)
	if((limit_bits & (1<<X_LIMIT_BIT)) ){
		result ^= STEPPER_INHIBIT_X;
	}
	
	if(flags & LIMIT_Y)
	if((limit_bits & (1<<Y_LIMIT_BIT)) ){
		result ^= STEPPER_INHIBIT_Y;
	}

	if(flags & LIMIT_Z)
	if((limit_bits & (1<<Z_LIMIT_BIT)) ){
		result ^= STEPPER_INHIBIT_Z;
	}
	
	if( (result & STEPPER_INHIBIT_ALL) == STEPPER_INHIBIT_ALL ){
		plan_set_current_position_n(pBlock->position[X_AXIS], pBlock->position[Y_AXIS], pBlock->position[Z_AXIS]);
	}
	
	return result;
}


void limits_go_home() {
  double x, y, z;

    // Store the current limit switch state
  uint8_t original_limit_state = LIMIT_PIN;

  st_synchronize();
  
  st_set_step_callback(&limit_step_callback);

  plan_get_current_position(&x, &y, &z);

	flags = LIMIT_Z;
	printPgmString(PSTR("homing z \n"));
	mc_line(x, y, z + 1000, 100, false);

	// wait for our block to complete
	st_synchronize();
	status_execute_line("?\n");

	plan_get_current_position(&x, &y, &z);

	flags = LIMIT_X | LIMIT_Y;
	printPgmString(PSTR("homing x y \n"));
	mc_line(x - 1000, y - 1000, z, 100, false);
	// wait for our block to complete
	st_synchronize();
	status_execute_line("?\n");
  
  // Xor previous and current limit switch state to determine which were high then but have become 
  // low now. These are the actual installed limit switches.
  uint8_t limit_switches_present = (original_limit_state ^ LIMIT_PIN) & LIMIT_MASK;
  

	// if at least one switch is installed move out of it
	if(limit_switches_present){
		flags = LIMIT_REVERSE;
		printPgmString(PSTR("homing reverse \n"));

		plan_get_current_position(&x, &y, &z);


		if(limit_switches_present & (1<<X_LIMIT_BIT)){
			x += 10;
			flags |= LIMIT_X;
		}

		if(limit_switches_present & (1<<Y_LIMIT_BIT)){
			y += 10;
			flags |= LIMIT_Y;
		}

		if(limit_switches_present & (1<<Z_LIMIT_BIT)){
			z -= 10;
			flags |= LIMIT_Z;
		}

		mc_line(x, y, z, 50, false);
		
		// wait for our block to complete
		st_synchronize();
		status_execute_line("?\n");
	}
	
	
st_set_step_callback(NULL);
}



// get limit switches; 1=closed, 0=open
void limits_get_limits(uint8_t* x, uint8_t* y, uint8_t* z){
	*x = ((LIMIT_PIN & LIMIT_MASK) & (1<<X_LIMIT_BIT)) > 0;
	*y = ((LIMIT_PIN & LIMIT_MASK) & (1<<Y_LIMIT_BIT)) > 0;
	*z = ((LIMIT_PIN & LIMIT_MASK) & (1<<Z_LIMIT_BIT)) > 0;
}
