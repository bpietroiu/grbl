/*
  print.c - Functions for formatting output strings
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

/* This code was initially inspired by the wiring_serial module by David A. Mellis which
   used to be a part of the Arduino project. */ 


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <avr/pgmspace.h>
#include "serial.h"

#ifndef DECIMAL_PLACES
#define DECIMAL_PLACES 3
#endif

void printString(const char *s)
{
	while (*s)
		serial_write(*s++);
}

// Print a string stored in PGM-memory
void printPgmString(const char *s)
{
  char c;
	while ((c = pgm_read_byte_near(s++)))
		serial_write(c);
}

void printIntegerInBase(unsigned long n, unsigned long base)
{ 
	unsigned char buf[8 * sizeof(long)]; // Assumes 8-bit chars. 
	unsigned long i = 0;

	if (n == 0) {
		serial_write('0');
		return;
	} 

	while (n > 0) {
		buf[i++] = n % base;
		n /= base;
	}

	for (; i > 0; i--)
		serial_write(buf[i - 1] < 10 ?
			'0' + buf[i - 1] :
			'A' + buf[i - 1] - 10);
}

void printInteger(long n)
{
	if (n < 0) {
		serial_write('-');
		n = -n;
	}

	printIntegerInBase(n, 10);
}

// A very simple 
void printFloat2(double number , uint8_t decimal_places)
{
	// Handle negative numbers
  if (number < 0.0)
  {
     serial_write('-');
     number = -number;
  }

  // Round correctly so that print(1.999, 2) prints as "2.00"
  double rounding = 0.5;
  uint8_t digits = decimal_places;
  uint8_t i;
  for ( i=0; i<digits; ++i)
    rounding /= 10.0;
  
  number += rounding;

  // Extract the integer part of the number and print it
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;
  printInteger(int_part);

  // Print the decimal point, but only if there are digits beyond
  if (digits > 0)
    serial_write('.'); 

  // Extract digits from the remainder one at a time
  while (digits-- > 0)
  {
    remainder *= 10.0;
    int toPrint = (int)(remainder);
    printInteger(toPrint);
    remainder -= toPrint; 
  } 
	  
//  double integer_part, fractional_part;
//  uint8_t decimal_part;
//  fractional_part = modf(n, &integer_part);
//  printInteger(integer_part);
//  serial_write('.');
//  fractional_part *= 10;
//  int decimals = DECIMAL_PLACES;  
//  while(decimals-- > 0) {
//    decimal_part = floor(fractional_part);
//    serial_write('0'+decimal_part);
//    fractional_part -= decimal_part;
//    fractional_part *= 10;
  //}
}


void printFloat(double number){
	 printFloat2(number, DECIMAL_PLACES);
}
