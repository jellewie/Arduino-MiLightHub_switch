#define Amount_Buttons 4
#define Time_StartLongPressMS 5000    //After howmuch MS we should consider a press a long press
#define Time_ESPrestartMS 10000       //After howmuch Ms we should restart the ESP, note this is only triggered on released, or on a CheckButtons() call

struct Pin_Switch {
  byte Button[Amount_Buttons];
  byte LED[Amount_Buttons];
};

struct Button_Time {
  byte Button;          //The button who has a updated state or is pressed (left blank if none)
  byte LED;             //The coresponding pin of the LED (left blank if none)
  bool StartPress;      //Triggered once on start press
  bool StartLongPress;  //Triggered once if timePressed > LongPress
  bool Pressed;         //if button is pressed
  bool PressedLong;     //if timePressed > LongPress
  int PressedTime;      //How long the button is pressed (in MS)
};

class Switch {
  private:                                //Private variables/functions
    bool ButtonLast[Amount_Buttons];
    bool ButtonLastLong[Amount_Buttons];
    bool ButtonCurrent[Amount_Buttons];
    unsigned long ButtonStartTime[Amount_Buttons];     	//Array with the button pressed time (to calculate long press)
    Pin_Switch Data;                          			//To store the pointer to the (group) of pins of this instance of buttons

  public:                                 //public variables/functions (these can be acces from the normal sketch)
    Switch(const Pin_Switch Input) {              		//Called to initialize
      this->Data = Input;								//Set the pointer, so we point to the pins
      for (byte i = 0; i < Amount_Buttons; i++) {	    //for each button
        pinMode(Data.Button[i], INPUT);   				//Set the button pin as INPUT
        if (Data.LED[i] != 0)             				//If a LED pin is given
          pinMode(Data.LED[i], OUTPUT);   				//Set the LED pin as output
      }
    }
    Button_Time CheckButtons() {
      Button_Time ReturnValue = {};
      for (byte i = 0; i < Amount_Buttons; i++) {       //for each button
        if (ButtonCurrent[i] or ButtonLast[i]) {        //If button is High (return pressed time) or needs updating (then update)
          ReturnValue.Button = i + 1;                   //Flag this button ID (start counting at 1)
          ReturnValue.LED = Data.LED[i];                //Flag the LED pin
          ReturnValue.PressedTime = millis() - ButtonStartTime[i];//Flag the pressed time
          if (ButtonCurrent[i])                         //If the button is pressed
            ReturnValue.Pressed = true;                 //Flag it's pressed (and not released)
          if (!ButtonLast[i])                           //If we just started pressing the button
            ReturnValue.StartPress = true;
          if (ReturnValue.PressedTime > Time_StartLongPressMS) { //if it was/is a long press
            if (ReturnValue.PressedTime > Time_ESPrestartMS)//if it was/is a way to long press
              ESP.restart();                            //Restart the ESP
            ReturnValue.PressedLong = true;             //Flag it's a long pres
            if (!ButtonLastLong[i]) {                   //If it's started to be a long press
              ReturnValue.StartLongPress = true;        //Flag that this was a long press
              ButtonLastLong[i] = true;
            }
          } else
            ButtonLastLong[i] = false;                  //Update last state
          ButtonLast[i] = ButtonCurrent[i];             //Update our known state (This is just used to later return 'Button released')
          return ReturnValue;                           //We can stop here, already found info to return
        }
      }
      return ReturnValue;
    }
    byte aButtonPressed() {
      //Returns the button states in bits; Like 0000<button1><b2><b3><b4> where 1 is HIGH and 0 is LOW
      //Example '00001001' = Buttons 1 and 4 are HIGH (Note we count from LSB)
      byte ButtonID = 0;
      for (byte i = 0; i < Amount_Buttons; i++) {
        ButtonID = ButtonID << 1;                     //Move bits 1 to the left (its like *2)
        if (digitalRead(Data.Button[i])) {
          ButtonID += 1;                              //Flag this button as on
          digitalWrite(Data.LED[i], !digitalRead(Data.LED[i]));
        } else
          digitalWrite(Data.LED[i], LOW);
      }
      return ButtonID;
    }
    void LEDsOff() {
      for (byte i = 0; i < Amount_Buttons; i++)
        digitalWrite(Data.LED[i], LOW);
    }
    void Pinchange(byte i) {
      //We do not need special overflow code here. Here I will show you with 4 bits as example
      //ButtonStartTime = 12(1100)    millis = 3(0011)    PressedTime should be = 7 (13,14,15,0,1,2,3 = 7 ticks)
      //PressedTime = millis() - ButtonStartTime[i] = 3-12=-9(1111 0111) overflow! = 7(0111)  Thus there is nothing to fix, it just works
      ButtonCurrent[i] = digitalRead(Data.Button[i]); //Write the state to mem
      if (ButtonCurrent[i])                           //If button is pressed
        ButtonStartTime[i] = millis();                //Save the start time
      else if (millis() - ButtonStartTime[i] > Time_ESPrestartMS) //If the button was pressed longer than 10 seconds
        ESP.restart();                                //Restart the ESP
    }
};
