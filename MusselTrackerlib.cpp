/* MusselTrackerlib.cpp
	Copyright Luke Miller 2015
	Library functions for the Mussel Tracker v2 boards. 
	Includes a class for Allegro A139x hall effect sensors
	and general functions.


*/


#include "MusselTrackerlib.h"

HallSensor::HallSensor(){}
HallSensor::~HallSensor(){}

// Function to initialize the input and output pins
// for the hall effect sensors. The specific pins that
// get activated are hard-coded here.
void HallSensor::begin(hallSensor_t hallsensor){
	switch(hallsensor){
		case HALL1:
			pinMode(A2, OUTPUT);
			digitalWrite(A2, LOW);
			pinMode(A6, INPUT);
			
			break;
		
		case HALL2:
			pinMode(A3, OUTPUT);
			digitalWrite(A3, LOW);
			pinMode(A7, INPUT);
			break;
	}
}

// Function to read a hall effect sensor. Call the function 
// using the name of the hall sensor define in the hallSensor typedef
// (either HALL1 or HALL2);
int HallSensor::readHall(hallSensor_t hallsensor){
	int readADC = 0;
	switch (hallsensor) {
		case HALL1:
			digitalWrite(A2, HIGH); // wake sensor
			delay(1); // allow time to wake up
			analogRead(A6); // make a throw-away reading
			delay(5);
			for (byte i = 0; i<4; i++){
				readADC = readADC + analogRead(A6);
				delay(5);
			}
			// Do a 2-bit right shift to divide readADC
			// by 4 to get the average of the 4 readings
			readADC = readADC >> 2;
			digitalWrite(A2, LOW); // sleep sensor
			return(readADC); // return value to calling function
		break;
		
		case HALL2:
			digitalWrite(A3, HIGH); // wake sensor
			delay(1); // allow time to wake up
			analogRead(A7); // make a throw-away reading
			delay(5);
			for (byte i = 0; i<4; i++){
				readADC = readADC + analogRead(A7);
				delay(5);
			}
			// Do a 2-bit right shift to divide readADC
			// by 4 to get the average of the 4 readings
			readADC = readADC >> 2;
			digitalWrite(A3, LOW); // sleep sensor
			return(readADC); // return value to calling function
		break;
	}
}
//------------------------------------------------------------
// General public functions

void printTimeSerial(DateTime now){
//------------------------------------------------
// printTime function takes a DateTime object from
// the real time clock and prints the date and time 
// to the serial monitor. 
	Serial.print(now.year(), DEC);
    Serial.print('-');
	if (now.month() < 10) {
		Serial.print(F("0"));
	}
    Serial.print(now.month(), DEC);
    Serial.print('-');
    if (now.day() < 10) {
		Serial.print(F("0"));
	}
	Serial.print(now.day(), DEC);
    Serial.print(' ');
	if (now.hour() < 10){
		Serial.print(F("0"));
	}
    Serial.print(now.hour(), DEC);
    Serial.print(':');
	if (now.minute() < 10) {
		Serial.print("0");
	}
    Serial.print(now.minute(), DEC);
    Serial.print(':');
	if (now.second() < 10) {
		Serial.print(F("0"));
	}
    Serial.print(now.second(), DEC);
	// You may want to print a newline character
	// after calling this function i.e. Serial.println();

}

//--------------------------------------------------------------
// printTimeToSD function. This formats the available data in the
// data arrays and writes them to the SD card file in a
// comma-separated value format.
void printTimeToSD (SdFile& mylogfile, DateTime tempTime) {
		// Write the date and time in a human-readable format
		// to the file on the SD card. 
		mylogfile.print(tempTime.year(), DEC);
		mylogfile.print(F("-"));
		if (tempTime.month() < 10) {
			mylogfile.print("0");
		}
		mylogfile.print(tempTime.month(), DEC);
		mylogfile.print(F("-"));
		if (tempTime.day() < 10) {
			mylogfile.print("0");
		}
		mylogfile.print(tempTime.day(), DEC);
		mylogfile.print(F(" "));
		if (tempTime.hour() < 10){
			mylogfile.print("0");
		}
		mylogfile.print(tempTime.hour(), DEC);
		mylogfile.print(F(":"));
		if (tempTime.minute() < 10) {
			mylogfile.print("0");
		}
		mylogfile.print(tempTime.minute(), DEC);
		mylogfile.print(F(":"));
		if (tempTime.second() < 10) {
			mylogfile.print("0");
		}
		mylogfile.print(tempTime.second(), DEC);
}

//-----------------------------------------------------------------------------
// goToSleep function. When called, this puts the AVR to
// sleep until it is awakened by an interrupt (TIMER2 in this case)
// This is a higher power sleep mode than the lowPowerSleep function uses.
void goToSleep() {
	// Create three variables to hold the current status register contents
	byte adcsra, mcucr1, mcucr2;
	// Cannot re-enter sleep mode within one TOSC cycle. 
	// This provides the needed delay.
	OCR2A = 0; // write to OCR2A, we're not using it, but no matter
	while (ASSR & _BV(OCR2AUB)) {} // wait for OCR2A to be updated
	// Set the sleep mode to PWR_SAVE, which allows TIMER2 to wake the AVR
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	adcsra = ADCSRA; // save the ADC Control and Status Register A
	ADCSRA = 0; // disable ADC by zeroing out the ADC status register
	sleep_enable();
	// Do not interrupt before we go to sleep, or the
	// ISR will detach interrupts and we won't wake.
	noInterrupts ();
	
	wdt_disable(); // turn off the watchdog timer
	
	//ATOMIC_FORCEON ensures interrupts are enabled so we can wake up again
	ATOMIC_BLOCK(ATOMIC_FORCEON) { 
		// Turn off the brown-out detector
		mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE); 
		mcucr2 = mcucr1 & ~_BV(BODSE);
		MCUCR = mcucr1; //timed sequence
		// BODS stays active for 3 cycles, sleep instruction must be executed 
		// while it's active
		MCUCR = mcucr2; 
	}
	// We are guaranteed that the sleep_cpu call will be done
	// as the processor executes the next instruction after
	// interrupts are turned on.
	interrupts ();  // one cycle, re-enables interrupts
	sleep_cpu(); //go to sleep
	//wake up here
	sleep_disable(); // upon wakeup (due to interrupt), AVR resumes here
	ADCSRA = adcsra; // re-apply the previous settings to the ADC status register

}
