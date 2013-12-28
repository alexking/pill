/*
 * Pill Shield
 * by Alex King (http://alexking.io)
 *
 * This is free and unencumbered software released into the public domain.
 * Visit http://unlicense.org for the full license.
 *
 */

// Include our library and dependancies
#include <SoftwareSerial.h>
#include <SmartThings.h>

// Define pins (RX / TX) - change depending on position of the D2/D3 <-> D0/D1 switch
const int smartthing_pins[] = { 3, 2};  // D2/D3
// const int smartthing_pins[] = {0, 1};  // D0/D1

// Initialize the SmartThings library
SmartThingsCallout_t handleMessage;
SmartThings smartthing(smartthing_pins[0], smartthing_pins[1], handleMessage);

// Button Pin
const int pillStatusPin = 12;

// Button Status 
boolean pillStatus;

boolean needToTakePills = false;
long asOfTime  = 0;

unsigned long minutesDisplayed = 0; 

// Minutes past the time we needed to take pills 
long minutesPast()
{

   return (millis() - asOfTime) / (1000ul * 60ul);
}

void asOfNow()
{
   asOfTime = millis(); 
}

void setup()
{

  // Setup button 
  pinMode(pillStatusPin, INPUT);
  pinMode(13, OUTPUT);
  
  Serial.begin(115200);
  Serial.println("Starting...");

  // Early check
  pillStatus = ( digitalRead(pillStatusPin) == HIGH );

  displayPillStatus(); 

  // Ask the server for an update 
  smartthing.send("update");
}

void handleMessage(String message)
{
  Serial.println(message);
  // Handle a message 
  if (message.equals("on"))
  {
    Serial.println("Turn on"); 
    smartthing.shieldSetLED(4, 0, 0); 

    // Note status and when it happened 
    needToTakePills = true; 
    asOfNow();
        
    // Confirm
    smartthing.send("on");

  } 
  else if (message.equals("off")) {

    Serial.println("Turn off");

    // Note status and when it happened 
    needToTakePills = false; 
    asOfNow(); 

    // Update display
    displayPillStatus();

    // Confirm that we are off 
    smartthing.send("off");

  } 
  else if (message.startsWith("set")) {

    Serial.println("Update is ");

    // Expect a format of
    // set:switch=on,ago=123,presence=open

      // Note delimeter     
    String delimeter = ","; 

    // Remove the set:
    String data = message.substring(4);

    // Keep track of our position
    int startFrom = 0;
    int foundAt = 0;

    // Work through 
    do 
    {
      foundAt = data.indexOf(delimeter, startFrom);

      // Seperate out the =
      String item = data.substring(startFrom, foundAt);
      int equals  = item.indexOf("=");
      handleUpdateFromServer(item.substring(0, equals), item.substring(equals + 1));

      startFrom = foundAt + 1; 

    } 
    while (foundAt != -1);

  }

}

// Parsed name value responses from the server (requested at startup)
// Called once per item (set:one=1,two=2 would result in two calls)
void handleUpdateFromServer(String name, String value)
{
  if (name == "switch")
  {
    
    if (value == "on")
    {
      needToTakePills = true; 
    } 
    else if (value == "off") {
      needToTakePills = false;
    }

  } else if (name == "contact") {

    if (value == "close")
    {
      pillStatus = true; 
    } 
    else if (value == "open") {
      pillStatus = false;
    }

  } else if (name == "ago") {

    // Set the as of time 
    asOfTime = millis() - ( value.toInt() * 1000ul * 60ul );
    
    Serial.print("Server return: "); 
    Serial.println(value);

    Serial.print("As Of Time: "); 
    Serial.println(asOfTime);

    Serial.print("Minutes past: "); 
    Serial.println(minutesPast());

  }



}

// Main loop 
void loop()
{
  // Read in data  
  smartthing.run(); 

  // Check pill status 
  boolean pillStatusCheck = ( digitalRead(pillStatusPin) == HIGH );

  // If they don't match, we'll need to send an update 
  if (pillStatus != pillStatusCheck)
  {
    // Update our variable 
    pillStatus = pillStatusCheck;
    
    if (pillStatus)
    {

      // Taken 
      if (needToTakePills)
      {
        needToTakePills = false;
        asOfNow();
      }

      // Close
      smartthing.send("close");

    } else {

      // Open
      smartthing.send("open");

    }

    displayPillStatus(); 

  }

  // If the pills are there and we need to take them 
  if (needToTakePills && pillStatus)
  {
    
    // Display this to the user (once per cycle, so it can blink and such)
    displayNeedToTakePills(); 

  }

}


void displayPillStatus()
{
    
  if (pillStatus)
  {
   
    // Green 
    smartthing.shieldSetLED(0, 1, 0); 

  } else {
    
    // Nice blue color 
    smartthing.shieldSetLED(0, 0, 1); 
  
  }
}


// Blink variables 
int blinkCounter = 0;
int blinkState   = 0; 

void displayNeedToTakePills()
{
  

    unsigned long minutes = minutesPast(); 
    if (minutes != minutesDisplayed)
    {
      minutesDisplayed = minutes; 
      
      if (minutes < 5)
      {
        smartthing.shieldSetLED(minutes + 1, 0, 0);
      }      
      
      Serial.println(minutesDisplayed);
    }
    
    blinkCounter++;
     
   if (minutes > 30)
   {
       if (blinkCounter > 100) {
         blinkState = ! blinkState;
         smartthing.shieldSetLED(blinkState * 5, 0, 0);
         blinkCounter = 0; 
      }

   } else if (minutes > 15) {
    
      if (blinkCounter > 300) {
         blinkState = ! blinkState;
         smartthing.shieldSetLED(blinkState * 5, 0, 0);
         blinkCounter = 0; 
      }

   } else if (minutes >= 5) {
    
      if (blinkCounter > 600) {
         blinkState = ! blinkState;
         smartthing.shieldSetLED(blinkState * 5, 0, 0);
         blinkCounter = 0; 
      }

   }

}


