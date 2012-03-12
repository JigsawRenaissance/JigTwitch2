/*
March 12th 2012
Twitch by Isa Farnik
Arduino based, tweeting light switch. For use in Jigsaw Renaissance (@Jigsawseattle).
This source from: https://github.com/JigsawRenaissance/JigTwitch2

Required Hardware:
1x Arduino Uno/SMD or Arduino Duemilanove w/ ATmega328
1x Arduino Ethernet Shield featureing the Wiznet 5100 chip
1x Houshold light switches
1x Appropriate length of terminated Cat5 cabling
1x Barrel negative power supply w/ at least 500mA of output

Hardware Description:
The first contact of the switch is connected to the Arduino's Gnd. The other contact,
connects both to one of the Arduino's digital I/O pins denoted by the variables switchPin,
as well as to a pull up/down resistor and which is then also connected to Gnd.

More info at http://wiki.jigren.org/Twitch
*/

#include <Ethernet.h>
#include <SPI.h>
#include <avr/pgmspace.h>

byte mac[] = { 0x4A, 0x69, 0x67, 0x73, 0x61, 0x77 }; // Jigsaw in Hex for the MAC address
char alixsysURL[] = "twitter.alixsys.com"; // twitter.alixsys.com/ 92, 243, 14, 243 IP of simpleAuth proxy service
IPAddress jigTwitchIP(192, 168, 0, 77);

const int switchPin = 8; // Light switch connected to pin 8
int switchState = digitalRead(switchPin); // Initial state of switch (off)

int reading = 0; // Used later during debounce code block

long lastDebounceTime = 0; // time in ms since last checked for debounce
long debounceDelay = 2500; // amount of time in ms (2.5 sec) to wait before sending off tweet

EthernetClient client;

const int tweetCount = 6; // total number of tweet strings, both on & off
int randomNumber = 42; // initialize random number
char buffer[200]; // character buffer used later

//char* debugTweet = " \\&_~!@#$%^&*()+QWERTYUIOP{}|[]-=`;:?/Well you made it this far..";

char* alixsysToken = "4e49c13d5a99d";  //4e49c13d5a99d:debugTwitch
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
  Serial.begin(9600);
  randomSeed(analogRead(0));

  pinMode(switchPin, INPUT);

  if (!Ethernet.begin(mac)) {
    // if DHCP fails, start with a hard-coded address:
    Ethernet.begin(mac, jigTwitchIP);
  }

  Serial.print("[DHCP] ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();

  randomNumber = random(0, (tweetCount-1));
  Serial.print("[RAND] New random number: ");
  Serial.println(randomNumber);
}

void loop() { 
  if (lastDebounceTime == 0) {
    reading = digitalRead(switchPin);
    if (reading != switchState) {
      lastDebounceTime = millis();
    };
  } 
  else if (((millis() - lastDebounceTime) >= debounceDelay) || (millis() < lastDebounceTime)) {
    reading = digitalRead(switchPin);
    if (reading != switchState) {
        Serial.print("[SWCH] Toggled, now ");
        Serial.print(reading);
        Serial.println(".");
        postToTwitter(tweetStrings[reading][randomNumber]);
    }
    switchState = reading;
    lastDebounceTime = 0;
  }
}

void postToTwitter(char *tweet) {
  sprintf(buffer,"GET /update/?axk=%s&status=%s HTTP/1.1\nHost: twitter.alixsys.com\n\n",alixsysToken,tweet);
  Serial.print("[TWIT] buffer: ");
  Serial.println(buffer);
  
  Serial.println("[HTTP] Connecting...");
  if (client.connect(alixsysURL, 80)) {
    Serial.println("[HTTP] Connected, sending HTTP GET");
    client.print(buffer);
    
    delay(debounceDelay);
    Serial.println("[HTTP] Checking for response...");
    if (client.available()) {
      Serial.println("[HTTP] Response found, printing.");
      char c = client.read();
      Serial.print(c);
      
      client.flush();
      Serial.println();
    } else {
      Serial.println("[HTTP] No response.");
    }
    client.stop();
  } else {
    Serial.println("[HTTP] Did not connect.");
  }
  Serial.println("[HTTP] Stopping connection.");
}

