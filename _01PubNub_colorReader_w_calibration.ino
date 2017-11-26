/*
 * Creation & Computation - Digital Futures, OCAD University
 * Kate Hartman / Nick Puckett
 * 
 * publish 2 random values to PubNub at a given interval
 *  
 * 
 * 
 */
#include <ArduinoJson.h> 
#include <SPI.h>

#include <WiFi101.h>
#define PubNub_BASE_CLIENT WiFiClient
#include <PubNub.h>

/*************************
 * color reader
 **************************/
 
//libraries for color reader
#include <Wire.h>
#include "Adafruit_TCS34725.h"

// set to false if using a common cathode LED
#define commonAnode true

// our RGB -> eye-recognized gamma color
byte gammatable[256];


Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

/*************************
 * color reader
 **************************/
 

static char ssid[] = "ocadu-embedded";      //SSID of the wireless network
static char pass[] = "internetofthings";    //password of that network
int status = WL_IDLE_STATUS;                // the Wifi radio's status

const static char pubkey[] = "pub-c-166b5eef-2e1a-45f4-8382-09a400ac380b";  //get this from your PUbNub account
const static char subkey[] = "sub-c-9cc5fc48-c883-11e7-9178-bafd478c18bc";  //get this from your PubNub account

const static char pubChannel[] = "channel1"; //choose a name for the channel to publish messages to


unsigned long lastRefresh = 0;
int publishRate = 2000;


int myValR;                       //variables to hold values to send
int myValG;
int myValB;

int rMax = 0;  // variables to calibrate
int rMin =255;
int bMax = 0;  // variables to calibrate
int bMin =255;
int gMax = 0;  // variables to calibrate
int gMin =255;

const int buttonPin = 6;     // the number of the pushbutton pin
int buttonState = 0;         // variable for reading the pushbutton status


void setup() 
{
  Serial.begin(9600);
  connectToServer();



  /*************************
 * v color reader v
 **************************/
    Serial.println("Color View Test!");

  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); // halt!
  }


    // thanks PhilB for this gamma table!
  // it helps convert RGB colors to what humans see
  for (int i=0; i<256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;
      
    if (commonAnode) {
      gammatable[i] = 255 - x;
    } else {
      gammatable[i] = x;      
    }
    //Serial.println(gammatable[i]);
  }

/*************************
 * ^ color reader 
 **************************/
  
} // end setup loop

void loop() 
{

    buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {      //calibrate sensor when button is pressed
    calibrate();
  } else {
    getSensorValue();           //look up what value is being pressed and then display it on the TFT screen
  }

  if(millis()-lastRefresh>=publishRate)   //timer used to publish the values at a given rate
  {
  publishToPubNub();
  lastRefresh=millis();   
  }
   

} //end loop loop

void getSensorValue(){
    
uint16_t clear, red, green, blue;

  tcs.setInterrupt(false);      // turn on LED

  delay(60);  // takes 50ms to read 
  
  tcs.getRawData(&red, &green, &blue, &clear);

  tcs.setInterrupt(true);  // turn off LED

    red= map(red,rMin,rMax,0,255); //adjust min and max value per color
  red = constrain(red,0,255);


  blue = map(blue,bMin,bMax,0,255);
  blue = constrain(blue,0,255);

  green = map(green,gMin,gMax,0,255);
  green = constrain(green,0,255);

  Serial.print("red ="); //code to verify in the serial in the values make sense
  Serial.print(red);
  Serial.print(" blue =");
  Serial.print(blue);   
  Serial.print(" green =");
  Serial.println(green);   

  myValR = red;
  myValG = green;
  myValB =  blue;
    
}

void calibrate() {
uint16_t clear, red, green, blue;

tcs.getRawData(&red, &green, &blue, &clear);

    // record the maximum sensor value
    if (red > rMax) {
      rMax = red;
    }
    // record the minimum sensor value
    if (red < rMin) {
      rMin = red;
    }

        if (blue > bMax) {
      bMax = blue;
    }
    // record the minimum sensor value
    if (blue < bMin) {
      bMin = blue;
    }

        if (green > gMax) {
      gMax = green;
    }
    // record the minimum sensor value
    if (green < gMin) {
      gMin = green;
    }
    Serial.print("rMax =");
    Serial.print(rMax);
    Serial.print("rMin =");
    Serial.print(rMin);
    
  }

void connectToServer()
{
  WiFi.setPins(8,7,4,2); //This is specific to the feather M0
 
  status = WiFi.begin(ssid, pass);                    //attempt to connect to the network
  Serial.println("***Connecting to WiFi Network***");


 for(int trys = 1; trys<=10; trys++)                    //use a loop to attempt the connection more than once
 { 
    if ( status == WL_CONNECTED)                        //check to see if the connection was successful
    {
      Serial.print("Connected to ");
      Serial.println(ssid);
  
      PubNub.begin(pubkey, subkey);                      //connect to the PubNub Servers
      Serial.println("PubNub Connected"); 
      break;                                             //exit the connection loop     
    } 
    else 
    {
      Serial.print("Could Not Connect - Attempt:");
      Serial.println(trys);

    }

    if(trys==10)
    {
      Serial.println("I don't think this is going to work");
    }
    delay(1000);
 }

  
}


void publishToPubNub()
{
  WiFiClient *client;
  StaticJsonBuffer<800> messageBuffer;                    //create a memory buffer to hold a JSON Object
  JsonObject& pMessage = messageBuffer.createObject();    //create a new JSON object in that buffer
  
 ///the imporant bit where you feed in values
  pMessage["randoValR"] = myValR;                      //add a new property and give it a value
  pMessage["randoValG"] = myValG;                     //add a new property and give it a value
  pMessage["randoValB"] = myValB;                     //add a new property and give it a value




///                                                       //you can add/remove parameter as you like
  
  //pMessage.prettyPrintTo(Serial);   //uncomment this to see the messages in the serial monitor
  
  
  int mSize = pMessage.measureLength()+1;                     //determine the size of the JSON Message
  char msg[mSize];                                            //create a char array to hold the message 
  pMessage.printTo(msg,mSize);                               //convert the JSON object into simple text (needed for the PN Arduino client)
  
  client = PubNub.publish(pubChannel, msg);                      //publish the message to PubNub

  if (!client)                                                //error check the connection
  {
    Serial.println("client error");
    delay(1000);
    return;
  }
  
  if (PubNub.get_last_http_status_code_class() != PubNub::http_scc_success)  //check that it worked
  {
    Serial.print("Got HTTP status code error from PubNub, class: ");
    Serial.print(PubNub.get_last_http_status_code_class(), DEC);
  }
  
  while (client->available()) 
  {
    Serial.write(client->read());
  }
  client->stop();
  Serial.println("Successful Publish");


  
}






