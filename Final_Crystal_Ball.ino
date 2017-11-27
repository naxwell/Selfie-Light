/*
 * Creation & Computation - Digital Futures, OCAD University
 * Max Lander + Roxanne Baril-Bedard
 * 
 *reads 3 values from pubnub and outputs them as RGB values for a neopixel featherwing
 *  
 *Adapted from original example code by Nick Puckett + Kate Hartman
 * 
 */
#include <ArduinoJson.h> 
#include <SPI.h>

#include <WiFi101.h>
#define PubNub_BASE_CLIENT WiFiClient
#include <PubNub.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(32, PIN, NEO_GRB + NEO_KHZ800);

static char ssid[] = "ocadu-embedded";      //SSID of the wireless network
static char pass[] = "internetofthings";    //password of that network
int status = WL_IDLE_STATUS;                // the Wifi radio's status

const static char pubkey[] = "pub-c-166b5eef-2e1a-45f4-8382-09a400ac380b";  //get this from your PUbNub account
const static char subkey[] = "sub-c-9cc5fc48-c883-11e7-9178-bafd478c18bc";  //get this from your PubNub account

const static char pubChannel[] = "trialsSub"; //choose a name for the channel to publish messages to
const static char subChannel[] = "channel1"; //choose a name for the channel to publish messages to


unsigned long lastRefresh = 0;
int publishRate = 1000;

boolean readWrite = false;

boolean messageReceived = false;


int yourValR;
int yourValG;
int yourValB;

int yourValRx;
int yourValGx;
int yourValBx;

int myVal1;
int myVal2;
int myVal3;


void setup() 
{
  Serial.begin(9600);
  connectToServer();
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(100);
}

void loop() 
{
  


  if(millis()-lastRefresh>=publishRate)
  {
  readFromPubNub();
   
  Serial.print("randoValR ");
  Serial.println(yourValR);
  
  Serial.print("randoValG ");
   Serial.println(yourValG);

   Serial.print("randoValB ");
   Serial.println(yourValB);


 
  lastRefresh=millis();   
  }

/* Attempt at manipulated values to display clearer colours */
//if(yourValR < 100){
// yourValRx = yourValR - 50;
//} else if (yourValR > 150){
// yourValRx = yourValR + 50;
//}
//if(yourValG < 100){
// yourValGx = yourValG - 50;
//} else if (yourValG > 150){
// yourValGx = yourValG + 50;
//}
//if(yourValB < 100){
// yourValBx = yourValB - 50;
//} else if (yourValB > 150){
// yourValBx = yourValB + 50;
//}
//Serial.println(yourValRx);
//Serial.println(yourValGx);
//Serial.println(yourValBx);

/* Print "HI" */
// strip.setPixelColor(0, 255, 0, 255);
//  strip.setPixelColor(8, 255, 0, 255);
//  strip.setPixelColor(16, 255, 0, 255);
//  strip.setPixelColor(24, 255, 0, 255);
//  strip.setPixelColor(1, 255, 0, 255);
//  strip.setPixelColor(9, 255, 0, 255);
//  strip.setPixelColor(17, 255, 0, 255);
//  strip.setPixelColor(25, 255, 0, 255);
//  strip.setPixelColor(10, 255, 0, 255);
//  strip.setPixelColor(18, 255, 0, 255);
//  strip.setPixelColor(3, 255, 0, 255);
//  strip.setPixelColor(11, 255, 0, 255);
//  strip.setPixelColor(19, 255, 0, 255);
//  strip.setPixelColor(27, 255, 0, 255);
//  strip.setPixelColor(4, 255, 0, 255);
//  strip.setPixelColor(12, 255, 0, 255);
//  strip.setPixelColor(20, 255, 0, 255);
//  strip.setPixelColor(28, 255, 0, 255);
//  strip.setPixelColor(6, 255, 0, 255);
//  strip.setPixelColor(14, 255, 0, 255);
//  strip.setPixelColor(22, 255, 0, 255);
//  strip.setPixelColor(30, 255, 0, 255);
//  strip.setPixelColor(7, 255, 0, 255);
//  strip.setPixelColor(15, 255, 0, 255);
//  strip.setPixelColor(23, 255, 0, 255);
//  strip.setPixelColor(31, 255, 0, 255);
//  strip.show();


//print received values to featherwith
 colorWipe(strip.Color(yourValR, yourValG, yourValB), 50);


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
      Serial.println("I don't this this is going to work");
    }
    delay(1000);
 }

  
}


/*void publishToPubNub()
{
  WiFiClient *client;
  StaticJsonBuffer<800> messageBuffer;                    //create a memory buffer to hold a JSON Object
  JsonObject& pMessage = messageBuffer.createObject();    //create a new JSON object in that buffer
  
 ///the imporant bit where you feed in values
  pMessage["val1"] = random(0,1000);                      //add the parameter "val1" to the object and give it a value
  pMessage["val2"] = random(101,200);                     //add the parameter "val2" to the object and give it a value

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
  
  if (PubNub.get_last_http_status_code_class() != PubNub::http_scc_success)
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


  
}*/



void readFromPubNub()
{
  StaticJsonBuffer<1200> inBuffer;                    //create a memory buffer to hold a JSON Object
  WiFiClient *sClient =PubNub.history(subChannel,1);

  if (!sClient) {
    Serial.println("message read error");
    delay(1000);
    return;
  }

  while (sClient->connected()) 
  {
    while (sClient->connected() && !sClient->available()) ; // wait
    char c = sClient->read();
    JsonObject& sMessage = inBuffer.parse(*sClient);
    
    if(sMessage.success())
    {
      //sMessage.prettyPrintTo(Serial); //uncomment to see the JSON message in the serial monitor
      yourValR = sMessage["randoValR"];  //
      Serial.print("randoValR ");
      Serial.println(yourValR);
      yourValG = sMessage["randoValG"];
      Serial.print("randoValG ");
      Serial.println(yourValG);
      yourValB = sMessage["randoValB"];
      Serial.print("randoValB ");
      Serial.println(yourValB);

      
    }
    
    
  }
  
  sClient->stop();

}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}




