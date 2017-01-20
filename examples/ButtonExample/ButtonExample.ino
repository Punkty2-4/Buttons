#include <Buttons.h>

// human readable context names for button ids
enum BUTTON_INDEX
{ 
	BUTTON_PLUS = 0,
	BUTTON_MINUS,
	BUTTON_CLEAR,
	BUTTON_STOP ,
	BUTTON_START,
	BUTTON_CONTROL,
	BUTTON_MAX
};

const unsigned int button_pins[]
{
		7,	
		6,
		8,
		9,
		12,
		13
};


Buttons buttons (BUTTON_MAX, button_pins);

int counter =0;



//--------------------------------------------------------------------------------------
// callback for when a button is pushed down
void buttonDown(byte input_id)
{
	Serial.print (input_id);
	Serial.println (" btn down");
	switch (input_id)
	{
	case BUTTON_START: 
		if (buttons.getButton (BUTTON_CONTROL))		// check for a persisent modifier key with multiple simultaneous button presses, like shift or control or alt.
			Serial.println ("Control + START!");
		else 
			Serial.println ("START!");
		break;
	case BUTTON_STOP: 
		Serial.println ("STOP!");
		break;
	}
}

//--------------------------------------------------------------------------------------
// callback for when a button is released 
void buttonUp(byte input_id)		
{
	Serial.print (input_id);
	Serial.println (" btn up");
	switch (input_id)
	{
	case BUTTON_PLUS:
		counter++;
		break;
	case BUTTON_MINUS:
		counter--;
		break;
	case BUTTON_CLEAR: 
		counter=0;
		break;
	default: 
		return;			// don't roll over into printing the current counter value
	}
	Serial.print ("COUNTER = ");
	Serial.println (counter);

}



void setup()
{
	Serial.begin (115200);
	Serial.println ("Button Example");
	if (buttons.begin()<1) 
		Serial.println ("Error starting buttons!");
	
	// set the callback functions
	buttons.buttonDown  (buttonDown);
	buttons.buttonUp  (buttonUp);
}



void loop()
{
	buttons.read();
}
