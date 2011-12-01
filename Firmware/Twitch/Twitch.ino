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
#include <EthernetDHCP.h> // DHCP library via gkaindl.com/software/arduino-ethernet/dhcp
#include <avr/pgmspace.h>

byte mac[] = { 0x4A, 0x69, 0x67, 0x73, 0x61, 0x77 }; // "Jigsaw" in Hex

byte alixsysIP[] = { 92, 243, 14, 243 };        // simpleauth -> Oauth proxy twitter.alixsys.com/ 92.243.14.243
//byte server[] = { 72, 2, 118, 214 };          // "                       " api.supertweet.net 72.2.118.214



const int switch0Pin = 8;        //Switch attached to pin 8

int switch0State = 0;
int reading = 0;

long lastDebounceTime = 0;
long debounceDelay = 2500;

Client twitter(alixsysIP, 80);          // Client object for alixsys oauth proxy on port 80

const int tweetCount = 6;               // Total number of tweets

int randomNumber = 42;

char buffer[200];                       // Buffer later used for strings


char* alixsysToken = "4d03053671de7"; //4e49c13d5a99d:debugTwitch - Tokens for the alixsys proxy for 2 twitter accounts
                                      //4d03053671de7:JigTwitch

prog_char *tweetStrings[2][tweetCount] = { // Tweet strings saved into the Arduino's flash (PROGram) MEMory, instead of SRAM
{
    "%23closed",            // URL escaped to save on space required by URL escaping function
    "Not%20Open.%20%23closed",
    "Door%27s%20locked.%20%23closed",
    "Nope.%20%23closed",
    "Cerrado.%20%23closed",
    "Time%20to%20go%20home%20for%20dinner.%20%23closed"}
  ,
  {                         // 2-D array loaded w/ tweet strings, col 0 as off, col 1 as on
    "Coffee%27s%20on.%20%23open",
    "%23open",
    "Come%20on%20over%21%20%23open",
    "Yes%2C%20We%27re%20open.%20%23open",
    "Someone%20is%20here%20%23open",
    "Door%20is%20unlocked.%20%23open"}
};

void setup() {
  delay(1000);              
  randomSeed(analogRead(0));            // required to keep random() different
  pinMode(switch0Pin, INPUT);
  EthernetDHCP.begin(mac);              // initialize dhcp
  randomNumber = random(0, (tweetCount-1));
}

void loop() { 
  if (lastDebounceTime == 0) {          // determine if debounce has been accounted for recently
    reading = digitalRead(switch0Pin);
    if (reading != switch0State) {
      lastDebounceTime = millis();      // begin debounce timer if switch flipped
    };
  } 
  else if (((millis() - lastDebounceTime) >= debounceDelay) || (millis() < lastDebounceTime)) {
    reading = digitalRead(switch0Pin);
    if (reading != switch0State) {              // if switch flipped since last checked
      randomNumber = random(0, (tweetCount-1));          // pick a random rumber to use as the tweet array index
      switch (reading) {                    // use switch state t determine which type of tweet
      case HIGH:
        postToTwitter(tweetStrings[reading][randomNumber]);
        break;
      case LOW:
        postToTwitter(tweetStrings[reading][randomNumber]);
        break;
      }
    }
    switch0State = reading;            // save temp switch state as default
    lastDebounceTime = 0;              // reset debounce timer
  }
  EthernetDHCP.maintain();             // maintain the DHCP lease
} 


void postToTwitter(char* tweet) {
  sprintf(buffer,"GET /update/?axk=%s&status=%s HTTP/1.1\nHost: twitter.alixsys.com\n\n",alixsysToken,tweet);  // use sprintf to concatanate HTTP request string
  if (twitter.connect()) {        // open connection to alixsys
    twitter.print(buffer);        // print HTTP
  }
    twitter.stop();
  } else if (!twitter.connect()) {        // must impliment better error handling..
    twitter.stop();
  }
}
