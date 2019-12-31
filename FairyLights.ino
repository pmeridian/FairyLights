#include <IRremote.h>

volatile byte mode = 0;
int brightness = 127;

bool initialised = 0;
bool timerOn = 0;

int flashCounter = 0;
int flashingSpeed = 30;
int glowingState = 0;
int glowingSpeed = 0;

unsigned long startTime=0;

#define OUTA 5
#define OUTB 6
#define BUTTON 2

#define MODES 2

const unsigned long ON_TIME=21600000;
const unsigned long DAY_TIME=86400000;
 
int RECV_PIN = 4;
IRrecv irrecv(RECV_PIN);
decode_results results;

#define MYDEBUG 0

void setBrightness(int value)
{
  if (value>127)
    value=127;
  if (value<0)
    value=0;
  if (value>0)
  {     
    //configure PWM TIMER (FastPWM, OUTB Inverted)
    //Use timer 0 (pin5 & 6 as IRrecv use TIMER2)
    TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(COM0B0) | _BV(WGM01) | _BV(WGM00);
    //TCCR0B = _BV(CS11); 
    OCR0A = value; 
    OCR0B = 255-value;
  }
  else
  {
     digitalWrite(OUTA,LOW);
     digitalWrite(OUTB,LOW);     
  }
}

void buttonPress()
{
  mode++;
  if (mode>MODES)
    mode=0;
}

void setup() {

#ifdef MYDEBUG
  Serial.begin(9600);
  // In case the interrupt driver crashes on setup, give a clue
  // to the user what's going on.
  Serial.println("Enabling IRin");
#endif

  //start at default mode always ON, max brightness
  pinMode(OUTA,OUTPUT);
  pinMode(OUTB,OUTPUT);
  pinMode(BUTTON,INPUT_PULLUP);
  pinMode(LED_BUILTIN,OUTPUT);
  //attach interrupt for button presses
  attachInterrupt(digitalPinToInterrupt(BUTTON), buttonPress, FALLING);

  mode=0;
  setBrightness(brightness);

  irrecv.enableIRIn(); // Start the receiver
#ifdef MYDEBUG
  Serial.println("Ready to go");
#endif
  digitalWrite(LED_BUILTIN,LOW);
}

void loop() {    
  delay(10);
  
  if (irrecv.decode(&results)) {
#ifdef MYDEBUG    
    Serial.println(results.value, HEX);
#endif
    if (results.value == 0xFF22DD)
      if (brightness>0)
      {
        mode=0;
        initialised=0;
      }
    if (results.value == 0xFFC23D)
      if (brightness>0)
      {
        mode=1;
        glowingSpeed=0;
      } 
    if (results.value == 0xFFE01F)
      if (brightness>0)
      { 
        mode=2;
        flashingSpeed=30;
      } 
    if (results.value == 0xFF5AA5)
    {
      if (mode==0)
        brightness+=20;
      if (mode==1)
        glowingSpeed+=1;
      if (mode==2)
        flashingSpeed+=10; 
    }
    if (results.value == 0xFF10EF)
    {
      if (mode==0)
        brightness-=20;
      if (mode==1)
        glowingSpeed-=1;
      if (mode==2)
        flashingSpeed-=10; 
    }
    if (results.value == 0xFFE21D)
    {
      mode=0;
      brightness=0;
      initialised=1; 
    }
    if (results.value == 0xFFA25D)
    {
      mode=0;
      brightness=127;
      initialised=1; 
    }
    if (results.value == 0xFF629D)
    {
      timerOn=1;
      startTime=millis();      
    }
    irrecv.resume(); // Receive the next value
  }

  unsigned long t=millis();
  if (timerOn && (t-startTime)>ON_TIME)
  {
    if ((t-startTime)>DAY_TIME)
      startTime=t;
    setBrightness(0);
    return;
  }
    
  if (mode==0)
  { 
    //Steady
    if (!initialised)
    {
      brightness=127;
      initialised=1;
    }
    if (brightness>127)
      brightness=127;
    if (brightness<0)
      brightness=0;
      
    setBrightness(brightness);
  }
  else if (mode==1)
  {
    //Glowing
    initialised=0;
    
    if (glowingSpeed<0)
      glowingSpeed=0;      
    
    if (brightness<=10)
        glowingState=1;
    if (brightness>=110)
        glowingState=0;
    
    flashCounter++;   

    if (glowingState==0)
      {
        if (flashCounter>glowingSpeed)
        {
          brightness=brightness-1;
          flashCounter=0;
        }
      }
    else
      { 
        if (flashCounter>glowingSpeed)
        {
          brightness=brightness+1;
          flashCounter=0;
        }
      }
    setBrightness(brightness); 
  }
  else if (mode==2)
  { 
    //Flashing
    initialised=0;
    if (flashingSpeed<30)
      flashingSpeed=30;
      
    if (flashCounter%flashingSpeed == 0)
    {
      setBrightness(127);
      flashCounter==0;
    }
    if (flashCounter%flashingSpeed == flashingSpeed/2)
      setBrightness(0);
    flashCounter++;
  }
}
