#if ARDUINO >= 100
#include "Arduino.h"
#include "Print.h"
#else
#include "WProgram.h"
#endif

#include "Buttons.h"


#define DEBUG 0


// fires the events and tracks button state changes
// returns 1 if the button event / state is changed from prior, 0 if the button state is the same
// and -1 for error
int Buttons::handleEvents( byte button, byte state )
{
	if (button > max_buttons ||  button < -1)
	{
#ifdef DEBUG
		Serial.println (F("BAD BUTTON "));
		Serial.println (button);
#endif
		return -1;
	}

	if (state > 0)	// there is a button pressed
	{
		if (!getButton(button))
		{
			setButton(button);
#ifdef DEBUG
			Serial.println (F("BUTTON DOWN"));
			Serial.println (button);
#endif
			if (button_down) button_down(button);		// fire event
			return 1;
		}
	}
	else
	{
			if (getButton(button))
			{
				clearButton(button);
#ifdef DEBUG
				Serial.println (F("BUTTON UP"));
				Serial.println (button);
#endif
				if (button_up) 
					button_up(button);		// fire event
				return 1;
			}
			clearButton(button);
	}
	return 0;
}


// -------------------------------------------------------------
// number of reads will set the number of times to read each digital input
// and then use the value that is > 1/2 of the reads 
void Buttons::read (int number_of_reads)
{
	if (pinslist==NULL) return;
	byte a,b;
	if (number_of_reads < 1)
		number_of_reads = 1;

	byte temp[max_buttons];
	memset (temp,0,max_buttons);
	for (b = 0; b < number_of_reads; b++)
	{
		for (a = 0; a < max_buttons; a++)
		{ 
			if (pinslist[a]<0) continue;				// handle invalid pins in pin mapping table.
			if (digitalRead(pinslist[a]) == 0) temp[a]++;
		} 
		// delay to do a little debouncing between each read cycle
		delay(DEBOUNCE_TIME_PER_READ);
	} 

	for (a = 0; a < max_buttons; a++)
	{
		byte is_now_on = temp[a] > (number_of_reads<<1);		// do not use >= because what if number_of_reads == 1 ??
		handleEvents (a,is_now_on);
	} 

}

// -------------------------------------------------------------
// number_of_reads controls how many times the analog input
// is read and averaged, which helps reduce noise
// however, there is a risk that if the timing is *just* right, 
// you could get a false button press that is somewhere between 
// the first and last analog reads.  We trap for that by seeing if 
// two consective reads of the analog pin are wildly different.
void AnalogButtons::read (int number_of_reads)
{
	if (number_of_reads < 1) number_of_reads = 1;
	unsigned long inp = 0;
	int a;
	int prev_read = analogRead(analog_pin);

	for (a = 0; a < number_of_reads; a++)
	{
		delay(5);
		int new_read = analogRead(analog_pin);
		if (abs(prev_read - new_read > 20))
		{
			delay(50);
			read(number_of_reads);		//  try to avoid major changes where we half of two states
			return;
		}
		inp += (new_read);
	};
	inp /= a;		// divide by read count to get average

#ifdef DEBUG
	Serial.print(F("\t\t"));
	Serial.println(inp);
	//  loggedval = inp;
#endif

	processReading(inp);

}
//-----------------------------------------------------------------------------------------------------------------
void AnalogButtons::processReading( unsigned long inp )
{
	int pressed;
	for (pressed = 0; pressed< max_buttons ; pressed++)
		if (inp < (thresholds_list[pressed]))
		{
#ifdef DEBUG
			Serial.print (inp);
			Serial.print (F(" is less than threshold "));
			Serial.print (thresholds_list[pressed]);
			Serial.print (F(" index #" ));
			Serial.print (pressed);
#endif
			for (int a = 0; a <= max_buttons; a++)
				handleEvents( pressed,pressed==a);
			return;
		}
}


