#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0x60, 0x8D, 0x17, 0x11, 0x23, 0x58 }; // 608D17
byte ip[] = { 192, 231, 221, 221 };
byte gateway[] = { 192, 231, 221, 129 };
byte subnet[] = { 255, 255, 255, 128 };

/*byte mac[] = { 0x00, 0x11, 0x23, 0x58, 0x13, 0x21 };
byte ip[] = { 192, 231, 221, 221 };
byte gateway[] = { 192, 231, 221, 129 };
byte subnet[] = { 255, 255, 255, 128 };*/

const char* ipToString(const uint8_t*);
byte twitterIP[] = { 92, 243, 14, 243 }; //twitter.alixsys.com/ 92.243.14.243

const int switch0Pin = 8;
//const int switch1Pin = 2;

int switch0State = 0;
int reading = 0;

long lastDebounceTime = 0;
long debounceDelay = 2500;
long loopCount = 0;

boolean firstLoop = true;

Client twitter(twitterIP, 80);
Client irc(jigbotIP, 11235);

const int tweetCount = 12;

int randomNumber = 42;
int randomNumberOld = 42;

char buffer[300];

//char* debugTweet = " \\&_~!@#$%^&*()+QWERTYUIOP{}|[]-=`;:?/Well you made it this far..";

char *tweetStrings[2][tweetCount] = {
  {
    "#closed",
    "Not Open. #closed",
    "Door's locked. #closed",
    "Nope. #closed",
    "Cerrado. #closed",
    "Time to go home for dinner. #closed",
    "Nobody's here!. #closed",
    "I'm sorry, I can't do that Dave. #closed",
    "Portal Closed. #closed",
    "Shut. #closed",
    "Worm Hole collapsed. #closed",
    "Le Clos-ed. #closed",       }
  ,
  {
    "Coffee's on. #open",
    "#open",
    "Come on over! #open",
    "Yes, We're open. #open",
    "Someone is here #open",
    "Door is unlocked. #open",
    "Abierto. #open",
    "Come in and play! #open",
    "Openinsky. #open",
    "Portal Open. #open",
    "Worm hole detected. #open",
    "Ooooooooooooopen. #open"       }
};

void setup() {

  Serial.begin(9600);

  pinMode(switch0Pin, INPUT); 

  Ethernet.begin(mac, ip, gateway, subnet);
  Serial.println("[ETHN] Ethernet started.");

  /*  if (twitter.connected()) {
   Serial.println("[HTTP] Twitter connected successfully");
   } 
   else {
   Serial.println("[HTTP] Twitter not connected.");
   }*/

  //Serial.println(urlEscape(debugTweet));
}

void loop() { 

  while (randomNumberOld == randomNumber) {
    randomNumber = random(0, (tweetCount-1));
    //    Serial.print("Old: ");
    //    Serial.println(randomNumberOld);
    //    Serial.print("New: ");
    //    Serial.println(randomNumber);
  }

  if (lastDebounceTime == 0) {
    reading = digitalRead(switch0Pin);
    if (reading != switch0State) {
      lastDebounceTime = millis();
    };
  } 
  else if (((millis() - lastDebounceTime) >= debounceDelay) || (millis() < lastDebounceTime)) {
    reading = digitalRead(switch0Pin);
    if (reading != switch0State) {
      switch (reading) {
      case HIGH:
        Serial.println("[SWCH] Left: On  - ");
                Serial.print(reading);
                Serial.print(" : ");
         Serial.println(tweetStrings[reading][randomNumber]);
                  sendIRCUpdate(tweetStrings[reading][randomNumber]);
        //          postToTwitter(tweetStrings[reading][randomNumber]);
        //        checkAvail();
        break;
      case LOW:
        Serial.println("[SWCH] Left: Off - ");
                Serial.print(reading);
                Serial.print(" : ");
         Serial.println(tweetStrings[reading][randomNumber]);
                sendIRCUpdate(tweetStrings[reading][randomNumber]);
        //        postToTwitter(tweetStrings[reading][randomNumber]);
        //        checkAvail();
        break;
      }
    }
    switch0State = reading;
    lastDebounceTime = 0;
    randomNumber = random(0, (tweetCount-1));
  }
} 

const char* ipToString(const uint8_t* ipAddr) {
  static char buf[16];
  sprintf(buf, "%d.%d.%d.%d\0", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
  return buf;
}

char* urlEscape(char* src) {

  static char bufferPickle[200];
  Serial.print("  src- ");
  Serial.println(sizeof(src));
    Serial.print("  buffer- ");

  String buffer = (String)src;
      Serial.println(sizeof(buffer));
  buffer = buffer.replace(" ", "%20");
  buffer = buffer.replace("&", "%26");
  buffer = buffer.replace("\\", "%5C");
  buffer = buffer.replace("#", "%23");

  buffer.toCharArray(bufferPickle, 200);
  Serial.print("  bufferPickle- ");
      Serial.println(sizeof(bufferPickle));
  return bufferPickle;
}

void postToTwitter(char* tweet) {

//  Serial.println("[TWIT] Got this far..");
  sprintf(buffer,"GET /update/?axk=4d1082d7e1a71&status=%s HTTP/1.1\nHost: twitter.alixsys.com\n\n",urlEscape(tweet));
//  Serial.print(buffer);
  if (twitter.connect()) {
 //   Serial.println("[TWIT] Twitter connected, sending GET request.");
    twitter.print(buffer);
 //   Serial.println("[TWIT] GET method sent.");
    twitter.stop();
  } 
/*  else if (!twitter.connect()) {
    Serial.println("[TWIT] Twitter failed to connect.");
    twitter.stop();
  }*/
}

void sendIRCUpdate(char* ircMessage) {
  
  ircMessage = urlEscape(ircMessage);
  Serial.print("[IRSI] ircMessage: ");
  Serial.println(ircMessage);
  sprintf(buffer,"GET /post?d=%s HTTP/1.1\nHost: irc.jigren.org\n\n",ircMessage);//urlEscape(
  Serial.print("[IRSI] buffer: ");
  Serial.println(buffer);
  
  if (irc.connect()) {
    Serial.println("[IRSI] IRC connected, sending GET method.");
    //Serial.print(buffer);
    Serial.println("[IRSI] GET method sent.");
    irc.stop();
  } 
  else if (!irc.connect()) {
    Serial.println("[IRSI] IRC failed to connect.");
    irc.stop();
  }
  
  /* if (irc.available()) {
    char c = irc.read();
    Serial.print(c);
  } */
}

/* void checkAvail()
 {
 if (twitter.connected())
 {
 if (twitter.available())
 {
 char c = twitter.read();
 Serial.print(c);
 }
 if (!twitter.connected())
 {
 Serial.println();
 Serial.println("[HTTP] Disconnecting.");
 twitter.stop();
 }
 }
 } */