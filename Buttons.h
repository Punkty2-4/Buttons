#pragma once
#if ARDUINO >= 100
#include "Arduino.h"
#include "Print.h"
#else
#include "WProgram.h"
#endif

// callback definition
typedef  void (*button_handler )(byte id);

const int DEBOUNCE_TIME_PER_READ = 5; // IN MS

// auto repeat is not well tested, use at your own risk

// wrapper class for digital buttons
// takes a button count and an array of pins 
// will watch them, track their status, generate button events for those pins, etc. 
class Buttons
{

    protected:
        unsigned int buttons;
        button_handler button_down;
        button_handler button_up;
         const unsigned int* pinslist;

        byte max_buttons ;
   		int handleEvents( byte button, byte state ); 	// the event dispatcher -- not meant to be called manually

    public:
		// button count is 16 max
        Buttons(byte button_count, const unsigned int* pins)
        {
            buttons = 0;
            max_buttons = button_count;
            button_down = NULL;
            button_up = NULL;
            pinslist  = pins;
		}

		// returns 0 on failure, 1 on success
        virtual int begin ()
        {
            int a;
			if (pinslist==NULL) return 0;
			clearAll();
            for (a = 0; a < max_buttons; a++)
                pinMode (pinslist[a], INPUT_PULLUP);
			return 1;

        }

		// get the status of a specific button last time it was read.
        bool getButton(int id) { return (buttons >> id) & 1; }
		bool AnyButtonPressed () { return buttons != 0;}

// handle reading buttons, and then dispatching events
		virtual void read (int number_of_reads = 1);
        
        
		// set callbacks for button events
        void buttonDown(button_handler val) { button_down = val; }
        void buttonUp(button_handler val) { button_up = val; }
		void setButton( int inp ) {  bitSet (buttons, inp);} ; 
		
		void clearButton( int inp ) { bitClear (buttons , inp); } 
        void clearAll( ) { buttons = 0; }

};


/****************************************************************************************************************************** 
 For analog buttons, we read a single analog input and compare it to an array of threshold values
 if the analog reading is less than a threshold value, then that array index is the button number
	buttons are assumed to be connected to pull down resistors, with a pull up resistor on the analog input
 a reading of 1023 means no buttons are pressed, as no button->pull down resistor is connected.

 the actual values will be determined by the voltage divider each button forms with the pull up and the button pull down.

 * Here is an example of button pull down resistor values, the voltage and associated ADC reading (threshold),
 * assuming 5V Aref and a 4700 ohm pull up resistor connected. 
	R		Volts 	ADC
	1000	0.88	179
	2200	1.59	326
	3300	2.06	422
	4700	2.50	512
	6800	2.96	605
	10000	3.40	696
	22000	4.12	843
	33000	4.38	895
	47000	4.55	930
	68000	4.68	957
	100000	4.78	977
  10000000	5.00	1023 (no button pushed, open circuit)

  So a reading of 0-178 means the first button is pushed. 179-325 will be the second button, etc. 

	BUT.... resistors are not exact -- they have tolerances, the thresholds need to adjusted to accomodate 

	let's say each resistor is +/- 10% -- that means the values become a low / high range: 

	----- Nominal------		---Worst case low ----   ---- Worst case max---
	R		Volts 	ADC		Low		LowV 	LowADC	Max		MaxV 	MaxADC
	1000	0.88	179		900		0.74	152		1100	1.03	211
	2200	1.59	326		1980	1.38	283		2420	1.82	372
	3300	2.06	422		2970	1.82	373		3630	2.31	472
	4700	2.50	512		4230	2.25	460		5170	2.75	563
	6800	2.96	605		6120	2.71	555		7480	3.19	653
	10000	3.40	696		9000	3.18	650		11000	3.61	739
	22000	4.12	843		19800	3.96	811		24200	4.26	871
	33000	4.38	895		29700	4.26	871		36300	4.48	916
	47000	4.55	930		42300	4.46	912		51700	4.62	946
	68000	4.68	957		61200	4.61	943		74800	4.73	968
	100000	4.78	977		90000	4.73	967		110000	4.81	985


	Notice that the values near the top end get increasingly close together...
	Starting with the 8th button, the low / high rangesactually overlap (worst case)...
	But at the low end of the table, we have plenty of separation. It would be better to change 
	the resistors to evenly space the values across the full ADC range
	(you need to know how many buttons you are using to do this) 

	All of this is a paint in the ass, but it does allow us to have multiple buttons on a single input pin

	And if you don't want to mess with all this math, you can wire up your buttons and run the sketch in 
	debug mode.  The analog button class will write the values out the serial port and you can simply 
	push each button in turn and write down the value.

	This class does NOT handle multiple simultaneous button presses.  That's a bit harder since you need to 
	make the pull down resistors distinguish much finer ranges to handle combinations. (halving / doubling resistor values
	for each step is a good starting point) 


	I recommend taking the table or debug reported values and adding like 5% to each threshold value,
	to accomodate noise /  voltage fluctation


	*/

class AnalogButtons : public Buttons
{
    protected:
        const unsigned int* thresholds_list;
        char analog_pin;
        bool pull_up;

		// this function does the special case processing of the analog buttons
		void processReading( unsigned long inp );

	public:

		// enable use_pull_up when you do not have an external pull up resistor for your button
		// thresholds array must have at least button_count values or we'll run off the end!
        AnalogButtons(int pin, int button_count, const unsigned int* thresholds, bool use_pull_up = false) : Buttons (button_count, NULL)
        {
            thresholds_list = thresholds;
            analog_pin = pin;
            pull_up = use_pull_up;
        }

		// returns 0 on failure, 1 on success
        virtual int begin ()
        {
			if (thresholds_list==NULL) return 0;
			pinMode (analog_pin, pull_up ? INPUT_PULLUP : INPUT);
			return 1;
        }

        virtual void read (int number_of_reads = 1);

        //-----------------------------------------------------------------------------------------------------------------
		// change the thresholds
        void setThresholds (const unsigned int* thresholds)
        {
            thresholds_list = thresholds;
        }
};

