/* MusselTrackerlib.h
	Luke Miller 2015
	Library functions for the Mussel Tracker v2 boards. 
	Includes a class for Allegro A1391 hall effect sensors
	and general functions.


*/

#ifndef MusselTrackerlib_H
#define MusselTrackerlib_H

#include <Arduino.h> // to get access to pinMode, digitalRead etc functions
#include "SdFat.h"
#include "RTClib.h" // to get access to DateTime class and functions

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


#endif