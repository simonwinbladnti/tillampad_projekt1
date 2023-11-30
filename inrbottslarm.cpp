/*
  File: innbrottsalarm.cpp
  Author: Simon Winblad
  Date: 2023-11-30
  Description: Fully functional system for recognising unauthorized precense
*/

#include <Keypad.h> // Aktivera bilbioteket för keypad och hämta dess funktioner

// Pins definition för enheter
const int redLamp = 12; // Rött larm-lampa
const int buzzer = 13; // Buzzer för ljudalarm
const int pingPin = 11; // Trigger-pin för ultraljudssensorn
const int echoPin = 10; // Echo-pin för ultraljudssensorn

// Variabler för avståndsmätning och alarmstatus
long duration;
long distance;
bool alarmStatus;
bool coolDown = false;
bool alarmPower = false;

// Längd och definition av lösenord (keypad)
const int passwordLength = 4;
char password[passwordLength + 1] = "1234"; // Fördefinierat lösenord
char enteredPassword[passwordLength + 1] = ""; // Input lösenord

// Definition av knappar på keypadden
const byte ROWS = 4; // Fyra rader på keypadden
const byte COLS = 4; 
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte colPins[COLS] = {2, 3, 4, 5}; // Kolumn-pins för keypadden
byte rowPins[ROWS] = {6, 7, 8, 9}; // Rad-pins för keypadden
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); // Sätt igång keypad med hjälp av bibliotekts funktion.

void setup() {
   Serial.begin(9600); 
   pinMode(redLamp, OUTPUT); // Starta pin för rött larm-lampa
   pinMode(buzzer, OUTPUT); // Starta pin för buzzer
}

void loop() {
  checkKeypad(); // Kontrollera keypadden innan andra operationer
  getDistance(); // Hämta avståndsdata från ultraljudssensorn
  checkDistance(); // Kontrollera avståndet för larm och cooldown
}

/*
  System for password input. Checks if the input of password matches the set password. Based on that activate or deactivate alarmpower.
  Parameters:
    - none (void)
  Returns: None
*/

void checkKeypad() {
  char button = customKeypad.getKey();

  if (button) {
    Serial.println(button);

    if (button == '*') { // Återställning av lösenord
      Serial.println("Password reset");
      memset(enteredPassword, 0, sizeof(enteredPassword)); // Återställ lösenord
    } else if (button == '#') { // Bekräftelseknapp
      if (strcmp(enteredPassword, password) == 0) { // Kontrollera om lösenord matchar det korrekta
        if (alarmPower) {
          Serial.println("Alarm deactivated");
          alarmPower = false;
        } else {
          Serial.println("Password correct - Alarm activated");
          alarmPower = true; // Aktivera larm
          memset(enteredPassword, 0, sizeof(enteredPassword)); // Återställ lösenord
        }
      } else {
        Serial.println("Incorrect password");
      }
    } else if (strlen(enteredPassword) < passwordLength) { // Kontrollera maximal längd för lösenord
      strncat(enteredPassword, &button, 1); // Lägg till tryckt knapp i skrivna lösenord
      Serial.println(enteredPassword); // Skriv ut lösenord för felsökning
    }
  }

  delay(10);
}


/*
  A function to get the distance between an object and sensor and changes global variable based on values.
  Parameters:
    - none (void)
  Returns: none
*/

void getDistance() {
   pinMode(pingPin, OUTPUT); // Bestäm vilken output
   digitalWrite(pingPin, LOW);
   delayMicroseconds(2); // Delay för kolla avstånd
   digitalWrite(pingPin, HIGH); // Aktivera ljudsignal
   delayMicroseconds(10);
   digitalWrite(pingPin, LOW); // Avaktivera ljudsignal
   pinMode(echoPin, INPUT); // Kolla tiden för att ljudsignal ska komma tillbaka
   duration = pulseIn(echoPin, HIGH); // Hämta mikrosekunder
   distance = microsecondsToCentimeters(duration); // Omvandla mikro sekunder till centimeter
}


/*
  System for activating the sound based on global variable activate
  Parameters:
    - Bool: activate
  Returns: None
*/

void activateSound(bool activate) {
  if (activate) {
    tone(buzzer, 400); // Aktivera piezo med tonfrekvens 400 Hz
    Serial.println("Buzzer activated");
  } else {
    noTone(buzzer); // Stäng av piezo
  }
}


/*
  Translate microseconds to centimeters.
  Parameters:
    - Long: microseconds
  Returns: Centimeters
*/

long microsecondsToCentimeters(long microseconds) {
   return microseconds / 29 / 2; // Omvandla mikrosekunder till centimeter
}


/*
  System for checking if alarm is able to activate or not based on the values of distance.
  Parameters:
    - none (void)
  Returns: None
*/

unsigned long lastAlarmTime = 0;
const int alarmCooldown = 60000; // 6 sekunder i millisekunder

void checkDistance() {
  unsigned long currentTime = millis();
  if (alarmPower) {
    if (!coolDown) {
      if (distance < 50) { // Om avståndet är mindre än 50 cm
        Serial.println("Alarm went off");
        digitalWrite(redLamp, HIGH); // Slå på lampan
        activateSound(true); // Aktivera ljudalarm (piezo)
        lastAlarmTime = currentTime; // Spara tiden för senaste larmet
        coolDown = true; // Sätt cooldown till sant
      } else {
        Serial.println("No Alarm");
        activateSound(false); // Inget larm, stäng av ljudalarm
        digitalWrite(redLamp, LOW); // Stänga av lampan
      }
    } else {
      if (currentTime - lastAlarmTime >= alarmCooldown) {
        Serial.println("Cooldown finished");
        coolDown = false; // Återställ cooldown efter 6 sekunder
      } else {
        // Håll den röda lampan på under cooldown-perioden
        digitalWrite(redLamp, HIGH);
      }
    }
  } else {
    digitalWrite(redLamp, LOW);
    activateSound(false);
    coolDown = false;
  }
}
