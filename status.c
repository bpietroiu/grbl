
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


// Parameter lines are on the form '$4=374.3' or '$' to dump current settings
uint8_t status_execute_line(char *line) {

  if(line[0] != '?') { 
    return(STATUS_UNSUPPORTED_STATEMENT); 
  }

	double x=0, y=0, z=0;
	plan_get_current_position(&x, &y, &z);
	
  if(line[1] != '?') { 
	
    printPgmString(PSTR("x="));
	printFloat2(x, 5);
    printPgmString(PSTR(" y="));
	printFloat2(y, 5);
    printPgmString(PSTR(" z="));
	printFloat2(z, 5);
    printPgmString(PSTR("\n"));
  }
  else{ // binary report
	int i;
	int8_t* ptr;
	ptr = (int8_t*)(&x);
	for(i=0;i<sizeof(double);i++){
		serial_write(ptr[i]);
	}
	ptr = (int8_t*)(&y);
	for(i=0;i<sizeof(double);i++){
		serial_write(ptr[i]);
	}
	ptr = (int8_t*)(&z);
	for(i=0;i<sizeof(double);i++){
		serial_write(ptr[i]);
	}
  }

  
  return(STATUS_OK);
}