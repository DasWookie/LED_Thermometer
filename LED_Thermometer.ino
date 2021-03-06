#include <dht.h>

int Debug = 0;

dht DHT;

// declare variables --------------------------------------
//int LEDs[10] = {0, 1, 3, 7, 15, 31, 63, 127, 255, 128};  // set number of LEDs to light up - BOTTOM UP
int LEDs[10] = {0, 128, 192, 224, 240, 248, 252, 254, 255, 1};  // set number of LEDs to light up - TOP DOWN

float tempRange[2] = {69.0, 75};  // set desired upper and lower Farenheit temps
float cutoff[8];  // array to store the intermediate steps between LED activations
int ledCount = 8;

int upperBank = 0;
int lowerBank = 0;
int samplePeriod = 2000; // set period for polling of temp sensor
int testDelay = 75;
int flickerCount = 4;

// define pins --------------------------------------------
int latchPin  =  8;  // connect to pin 12(STcp) connect to both 595 chips
int dataPin   = 11;  // connect to pin 14(Ds)
int clockPin  = 12;  // connect to pin 11(SHcp) connect to both 595 chips
int tempPin   =  9;  // connect to center pin on TMP36

void setup()
{
// define inputs/outputs
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(tempPin, INPUT);

  Serial.begin(9600);
  Serial.println("DHT TEST PROGRAM ");
  Serial.print("LIBRARY VERSION: ");
  Serial.println(DHT_LIB_VERSION);

  selfTest();

// define the temperature cutoff increment points for the LEDs
  float incr =((float)tempRange[1] - (float)tempRange[0]) / (float)(ledCount); // number of steps between total LEDs
  Serial.print("Temp range: Low - ");
  Serial.println(tempRange[0]);
  Serial.print("Temp range: Hi  - ");
  Serial.println(tempRange[1]);
  Serial.print("Temp Increments:");
  Serial.println(incr);

  for (int i=0; i < ledCount; i++) {
    cutoff[i] = tempRange[0] + (incr * (float)i);
    Serial.print("Temp range step ");
    Serial.print(i);
    Serial.print(":\t");
    Serial.println(cutoff[i]);
  }
}


void loop()
{
  //int reading = analogRead(tempPin);        // read temperature probe voltage
  int chk = DHT.read22(tempPin);

  switch (chk)
  {
  case DHTLIB_OK:
      Serial.print("OK,\t");
      break;
  case DHTLIB_ERROR_CHECKSUM:
      Serial.print("Checksum error,\t");
      break;
  case DHTLIB_ERROR_TIMEOUT:
      Serial.print("Time out error,\t");
      break;
  default:
      Serial.print("Unknown error,\t");
      break;
  }

  // DISPLAY DATA
  Serial.print(DHT.humidity, 1);
  Serial.print(",\t");
  Serial.print(DHT.temperature, 1);
  Serial.print(",\t");
  Serial.println();


  float tempC = DHT.temperature;
  float tempF = (tempC * 9.0)/5.0 + 32.0 ;

  if (Debug == 1) {
    tempF = 78;
  }

  Serial.print("Temp = ");  // send temperature to serial
  Serial.println(tempF);

// assign byte values to be sent to LEDs
  
  if(tempF < tempRange[0])  // Warn if out of range (COLD)
    {
      for (int i = 1; i <= flickerCount; i++) {
        lowerBank = LEDs[1];
        sendTemp();
        delay(testDelay);
        lowerBank = LEDs[0];
        sendTemp();
        delay(testDelay);
      }
      lowerBank = LEDs[0];
    }
  else if(tempF >= tempRange[0] && tempF < cutoff[1]) //  Set 1-16 LEDs to light up depending on temp
    {
      lowerBank = LEDs[1];
    } 
  else if(tempF >= cutoff[1] && tempF < cutoff[2])  
    {
      lowerBank = LEDs[2];
    }  
  else if(tempF >= cutoff[2] && tempF < cutoff[3])
    {
      lowerBank = LEDs[3];
    }
  else if(tempF >= cutoff[3] && tempF < cutoff[4])
    {
      lowerBank = LEDs[4];
    }
  else if(tempF >= cutoff[4] && tempF < cutoff[5])
    {
      lowerBank = LEDs[5];
    }
  else if(tempF >= cutoff[5] && tempF < cutoff[6])
    {
      lowerBank = LEDs[6];
    }
  else if(tempF >= cutoff[6] && tempF < cutoff[7])
    {
      lowerBank = LEDs[7];
    }    
  else if(tempF >= cutoff[7] && tempF < tempRange[1])
    {
      lowerBank = LEDs[8];
    }
  else if(tempF > tempRange[1]) //  Warn if out of range (HOT)
    {
      for (int i = 1; i <= flickerCount; i++) {
        lowerBank = LEDs[8];
        sendTemp();
        delay(testDelay);
        lowerBank = LEDs[7];
        sendTemp();
        delay(testDelay);
      }
      lowerBank = LEDs[7];
    }

  sendTemp();           // send to LEDs function
  delay(samplePeriod);  // wait awhile for next update
}


// Send bytes to 74HC595 chips to light LEDs
void sendTemp()
{
  digitalWrite(latchPin, LOW);                      // latch LOW to send data
  shiftOut(dataPin, clockPin, MSBFIRST, lowerBank); // send byte for LEDs 9-16(shift upper)
  digitalWrite(latchPin, HIGH);                     // latch HIGH to stop data transfer
}

void selfTest() {
  // run a sequential LED test
  for (int i=0; i < flickerCount; i++) {
    for (int j=0; j <= ledCount; j++)  // lower + upper bank
    {
      lowerBank = LEDs[j];
      sendTemp();
      delay(testDelay);
    }
  }
  // Blink all LED test
  for (int i=0; i < flickerCount; i++) {
      delay(testDelay * 2);
      lowerBank = LEDs[0];
      sendTemp();
      delay(testDelay * 2);
      lowerBank = LEDs[8];
      sendTemp();
  }
  // keep all LEDs lit for a bit, then turn them all off
  delay(testDelay * 2);
  lowerBank = LEDs[0];
  sendTemp();
  delay(samplePeriod);
}
