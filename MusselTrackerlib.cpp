/* MusselTrackerlib.cpp
	Copyright Luke Miller 2015
	Library functions for the Mussel Tracker v2 boards. 
	Includes a class for Allegro A139x hall effect sensors
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


#include "MusselTrackerlib.h"
#include "LSM303.h"

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
			delay(2);
			for (byte i = 0; i<4; i++){
				readADC = readADC + analogRead(A6);
				delay(2);
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
			delay(2);
			for (byte i = 0; i<4; i++){
				readADC = readADC + analogRead(A7);
				delay(2);
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


//-----------------------------------------------------------------------------
// lowPowerSleep function
// This sleep version uses the watchdog timer to sleep for 8 seconds at a time

void lowPowerSleep(void){

	/* It seems to be necessary to zero out the Asynchronous clock status 
	 * register (ASSR) before enabling the watchdog timer interrupts in this
	 * process. 
	 */
	ASSR = 0;  
	TIMSK2 = 0; // stop timer 2 interrupts
	// Cannot re-enter sleep mode within one TOSC cycle. 
	// This provides the needed delay.
	OCR2A = 0; // write to OCR2A, we're not using it, but no matter
	while (ASSR & _BV(OCR2AUB)) {} // wait for OCR2A to be updated

	ADCSRA = 0;   // disable ADC
	set_sleep_mode (SLEEP_MODE_PWR_DOWN);  // specify sleep mode
	sleep_enable();
	// Do not interrupt before we go to sleep, or the
	// ISR will detach interrupts and we won't wake.
	noInterrupts ();
	//--------------------------------------------------------------------------
	// Set up Watchdog timer for long term sleep

	// Clear the reset flag first
	MCUSR &= ~(1 << WDRF);

	// In order to change WDE or the prescaler, we need to
	// set WDCE (This will allow updates for 4 clock cycles).
	WDTCSR |= (1 << WDCE) | (1 << WDE);
	// Enable watchdog interrupt (WDIE), and set 8 second delay
	WDTCSR = bit(WDIE) | bit(WDP3) | bit(WDP0); 
	wdt_reset();

	// Turn off brown-out enable in software
	// BODS must be set to one and BODSE must be set to zero within four clock 
	// cycles, see section 10.11.2 of 328P datasheet
	MCUCR = bit (BODS) | bit (BODSE);
	// The BODS bit is automatically cleared after three clock cycles
	MCUCR = bit (BODS);
	// We are guaranteed that the sleep_cpu call will be done
	// as the processor executes the next instruction after
	// interrupts are turned on.
	interrupts ();  // one cycle, re-enables interrupts
	sleep_cpu ();   // one cycle, going to sleep now, wake on interrupt
	// The AVR is now asleep. In SLEEP_MODE_PWR_DOWN it will only wake
	// when the watchdog timer counter rolls over and creates an interrupt
	//-------------------------------------------------------------------
	// disable sleep as a precaution after waking
	sleep_disable();
}

//-------------- checkMCUSR ------------------------------------------------
// Function checkMCUSR(byte mcusr, byte ERRLED)
// This function is supplied a byte value that represents the contents of the
// MCU status register. On a restart, this register's 4 lowest bits contain
// flags noting what might have caused the restart. If the restart was due
// to a brown-out, we want to go into a permanent sleep mode to avoid 
// corrupting the SD card. 
void checkMCUSR(byte mcusr, byte ERRLED) 
{	
	//----------------------------------------------------------------------
	// Check the MCU Status Register (MCUSR) to see why the
	// program has started/restarted. See section 11.9.1 of the AVR datasheet.
	// Check if the Brown-Out Reset Flag is 1 by &-ing MCUSR with a 1 in
	// the BORF position. MCUSR and BORF are defined internally in the
	// various avr libraries. 
	if (mcusr & _BV(BORF) ) { 
		// Check to see if the external reset flag EXTRF was set. If it 
		// is set, this next test will fail and normal bootup will proceed.
		if (!(mcusr & _BV(EXTRF))){	
			// Now check if the Power-On Reset Flag is 0. A full power-on
			// reset will usually have both BORF and PORF true, but a 
			// brown-out when already powered-up will not have PORF true. 
			if (!(mcusr & _BV(PORF))){ 
				// Uh-oh, PORF was not true, so we only have a BORF flag.
				// This likely signals that the power supply is getting low
				// enough that it is hitting the brown-out level (~2.7v).
				// In this case, we should put the unit into a permanent
				// sleep to avoid corrupting the SD card.

				// Go into a permanent sleep loop. This cannot be exited 
				// without a reset of some sort. Continued brown-outs should
				// simply end up back here. The ERRLED should blink briefly
				// every 8 seconds to indicate that the program has ended 
				// up in this brown-out-induced loop.
				while(1){
					digitalWrite(ERRLED, HIGH);
					delay(3);
					digitalWrite(ERRLED, LOW);
					lowPowerSleep();  // go back to sleep
				}
			}
		}
	}
}

//-------------- printBits -----------------------------------------
// A simple function to print out a byte as 1's and 0's to Serial
void printBits(byte myByte){
	for(byte mask = 0x80; mask; mask >>= 1){
		if(mask  & myByte){
		   Serial.print('1');
		} else {
		   Serial.print('0');
		}
	}
}