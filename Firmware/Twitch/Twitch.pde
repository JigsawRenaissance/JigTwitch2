/*
November 26th 2011
Twitch by Isa Farnik
Arduino based, tweeting light switch. For use in Jigsaw Renaissance (@Jigsawseattle).
This source from: https://github.com/JigsawRenaissance/JigTwitch2

Required Hardware:
1x Arduino Uno/SMD or Arduino Duemilanove w/ ATmega328
1x Arduino Ethernet Shield featureing the Wiznet 5100 chip
2x Houshold light switches
1x Plastic or otherwise 
1x Appropriate length of terminated Cat5 cabling
1x Barrel negative power supply w/ at least 500mA of output

Hardware Description:
The first contact of both switches is connected to the Arduino's Gnd. The other contact,
connects both to one of the Arduino's digital I/O pins denoted by the variables switch0Pin
& switch1Pin, as well as to a pull up/down resistor and which is then also connected to Gnd.

More info at http://wiki.jigren.org/Twitch
*/

#if defined(ARDUINO) && ARDUINO > 18   // Arduino 0019 or later
#include <SPI.h>
#endif
#include <Ethernet.h>
#include <EthernetDHCP.h>
#include <avr/pgmspace.h>

byte mac[] = { 0x4A, 0x69, 0x67, 0x73, 0x61, 0x77 }; // Jigsaw in Hex

const char* ipToString(const uint8_t*);
byte alixsysIP[] = { 92, 243, 14, 243 }; //twitter.alixsys.com/ 92.243.14.243
//byte server[] = { 72, 2, 118, 214 }; //api.supertweet.net 72.2.118.214
//byte freenodeIP[] = { 130, 237, 188, 200 }; //freenode.net
//byte jigbotIP[] = { 205, 201, 45, 29 }; //http://irc.jigren.org 205.201.45.29

const int switch0Pin = 8;
//const int switch1Pin = 2;

int switch0State = 0;
int reading = 0;

long lastDebounceTime = 0;
long debounceDelay = 2500;
long loopCount = 0;

boolean firstLoop = true;

Client twitter(alixsysIP, 80);

const int tweetCount = 6;

int randomNumber = 42;

char buffer[200];

//char* debugTweet = " \\&_~!@#$%^&*()+QWERTYUIOP{}|[]-=`;:?/Well you made it this far..";

char* alixsysToken = "4d03053671de7"; //4e49c13d5a99d:debugTwitch
                     //4d03053671de7:JigTwitch

prog_char *tweetStrings[2][tweetCount] = {
//{"#closed","Not Open. #closed","Door's locked. #closed","Nope. #closed","Cerrado. #closed","Time to go home for dinner. #closed","Nobody's here!. #closed","I'm sorry, I can't do that Dave. #closed","Portal Closed. #closed","Shut. #closed","Worm Hole collapsed. #closed","Le Clos-ed. #closed",   }  ,  {"Coffee's on. #open","#open","Come on over! #open","Yes, We're open. #open","Someone is here #open","Door is unlocked. #open","Abierto. #open","Come in and play! #open","Openinsky. #open","Portal Open. #open","Worm hole detected. #open","Ooooooooooooopen. #open"}
//{"%23closed","Not%20Open.%20%23closed","Door%27s%20locked.%20%23closed","Nope.%20%23closed","Cerrado.%20%23closed","Time%20to%20go%20home%20for%20dinner.%20%23closed","Nobody%27s%20here%21.%20%23closed","I%27m%20sorry%2C%20I%20can%27t%20do%20that%20Dave.%20%23closed","Portal%20Closed.%20%23closed","Shut.%20%23closed","Worm%20Hole%20collapsed.%20%23closed","Le%20Clos-ed.%20%23closed"}};
{
    "%23closed",
    "Not%20Open.%20%23closed",
    "Door%27s%20locked.%20%23closed",
    "Nope.%20%23closed",
    "Cerrado.%20%23closed",
    "Time%20to%20go%20home%20for%20dinner.%20%23closed"}
  ,
  {
    "Coffee%27s%20on.%20%23open",
    "%23open",
    "Come%20on%20over%21%20%23open",
    "Yes%2C%20We%27re%20open.%20%23open",
    "Someone%20is%20here%20%23open",
    "Door%20is%20unlocked.%20%23open"}
};

void setup() {
  delay(1000);
  //Serial.begin(9600);
  randomSeed(analogRead(0));

  pinMode(switch0Pin, INPUT); 

  EthernetDHCP.begin(mac);
  //Serial.println("[DHCP] Ethernet started.");

  const byte* ipAddr = EthernetDHCP.ipAddress();
  const byte* gatewayAddr = EthernetDHCP.gatewayIpAddress();
  const byte* dnsAddr = EthernetDHCP.dnsIpAddress();

  //Serial.print("[DHCP] My IP address is ");
  //Serial.println(ipToString(ipAddr));
  ////Serial.print("[DHCP] Gateway IP address is ");
  ////Serial.println(ipToString(gatewayAddr));
  ////Serial.print("[DHCP] DNS IP address is ");
  ////Serial.println(ipToString(dnsAddr));

  ////Serial.println(urlEscape(debugTweet));

  randomNumber = random(0, (tweetCount-1));
  //Serial.print("[RAND] New random number: ");
  //Serial.println(randomNumber);
}

void loop() { 
  
  if (lastDebounceTime == 0) {
    reading = digitalRead(switch0Pin);
    if (reading != switch0State) {
      lastDebounceTime = millis();
    };
  } 
  else if (((millis() - lastDebounceTime) >= debounceDelay) || (millis() < lastDebounceTime)) {
    reading = digitalRead(switch0Pin);
    if (reading != switch0State) {
      randomNumber = random(0, (tweetCount-1));
      //Serial.print("[RAND] New random number: ");
      //Serial.println(randomNumber);
      switch (reading) {
      case HIGH:
        //Serial.println("[SWCH] Left: On");
        
        postToTwitter(tweetStrings[reading][randomNumber]);
        //        checkAvail();
        break;
      case LOW:
        //Serial.println("[SWCH] Left: Off");
        postToTwitter(tweetStrings[reading][randomNumber]);
        //        checkAvail();
        break;
      }
    }
    switch0State = reading;
    lastDebounceTime = 0;
  }
  EthernetDHCP.maintain();
} 

const char* ipToString(const uint8_t* ipAddr) {
  static char buf[16];
  sprintf(buf, "%d.%d.%d.%d\0", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
  return buf;
}

/* char* urlEscape(char* src) {

  static char bufferPickle[200];
  String buffer = src;
  //Serial.print("[URLE] src: ");
  //Serial.println(src);
  //Serial.print("[URLE] buffer: ");
  //Serial.println(buffer);
  buffer = buffer.replace(" ", "%20");
  buffer = buffer.replace("&", "%26");
  buffer = buffer.replace("\\", "%5C");
  buffer = buffer.replace("#", "%23");
  buffer = buffer.replace("!", "%21");
  buffe
  r = buffer.replace("'", "%27");
  buffer = buffer.replace(",", "%2C");

  buffer.toCharArray(bufferPickle, 200);
  //Serial.print("[URLE] bufferPickle- ");
  
      //Serial.println(sizeof(bufferPickle));
  return bufferPickle;
} */

void postToTwitter(char* tweet) {

  //Serial.print("[TWIT] ");
  //Serial.println((String)tweet);
  //tweet = urlEscape(tweet);
  
  ////Serial.println("[TWIT] Got past urlEscape(tweet)");
  sprintf(buffer,"GET /update/?axk=%s&status=%s HTTP/1.1\nHost: twitter.alixsys.com\n\n",alixsysToken,tweet);
  //Serial.print("[TWIT] buffer: ");
  //Serial.println(buffer);
  ////Serial.println("[TWIT] Got past sprintf");
  if (twitter.connect()) {
    //Serial.println("[TWIT] Twitter connected, sending GET request.");
    twitter.print(buffer);
    
    //Serial.println("[TWIT] GET method sent.");
    if (twitter.available()) {
      char c = twitter.read();
      //Serial.print(c);
    
      //twitter.flush();
    }
    twitter.stop();
  } else if (!twitter.connect()) {
    //Serial.println("[TWIT] Twitter failed to connect.");
    twitter.stop();
  }
}