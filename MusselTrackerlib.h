/* MusselTrackerlib.h
	Copyright Luke Miller 2015
	Library functions for the Mussel Tracker v2 boards. 
	Includes a class for Allegro A1391 hall effect sensors
	and general functions.
	
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


*/

#ifndef MusselTrackerlib_H
#define MusselTrackerlib_H

#include <Arduino.h> // to get access to pinMode, digitalRead etc functions
#include "SdFat.h"	// https://github.com/greiman/SdFat
#include "RTClib.h" // https://github.com/millerlp/RTClib
#include "LSM303.h"	// https://github.com/pololu/lsm303-arduino
// Various additional libraries for access to sleep mode functions
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/atomic.h>
#include <wiring_private.h>
#include <avr/wdt.h>

// Create a variable type hallSensor_t that can 
// take on 2 values, HALL1 or HALL2. This allows 
// these two keywords to be passed to the HallSensor class
// functions to specify which sensor you want to address.
enum hallSensor_t
{
  HALL1,
  HALL2
};

class HallSensor {
	public:
		HallSensor();
		~HallSensor();
		void begin(hallSensor_t hallsensor); // set up pinMode for the sensor
		int readHall(hallSensor_t hallsensor); // returns A-D value from sensor
};

//----------------------------------------------------------------------------------
// General public functions to use in the MusselTracker programs

// Print formatted Date + Time to Serial monitor
void printTimeSerial(DateTime now); 

// Print formatted Date + Time to SD card csv file. Notice that this passes the
// SdFile object by reference (SdFile& mylogfile) instead of making a copy and passing by value
// (which SdFile mylogfile would do).
void printTimeToSD(SdFile& mylogfile, DateTime now); 

// Put the AVR to sleep until a TIMER2 interrupt fires to awaken it
void goToSleep();

// Put the AVR into a low power sleep mode with 8 second delays
// Used primarily if you're done taking data
void lowPowerSleep();

// Check the contents of the MCU Status Register and put the 
// chip into permanent low power sleep if it has suffered a 
// brownout (~2.7V). 
void checkMCUSR(byte mcusr, byte ERRLED);

// A simple function to print out a byte as 1's and 0's to Serial
void printBits(byte myByte);

#endif