/*
  Weasley Clock 
  -------------

  Created by Svyatoslav Polishchuk
  
  Inspiration       Joan K Rowling (https://www.pinterest.com/pin/313492824048825583/)
  Based on code     Doug "LegoDoug" Wilcox (http://blog.wilcoxfamily.net/2016/12/weasley-clock-code/)
                    Alec Moore (Adafruit MQTT Library: Ethernet example – https://goo.gl/gTXs1x)
  
*/

/* =====================         Include libraries      =====================*/

#include <SPI.h>                                            // Arduino Serial Peripheral Interface library
#include <Dns.h>                                            // Arduino DNS library
#include <Dhcp.h>                                           // Arduino DHCP library
#include <Ethernet.h>                                       // Arduino Ethernet library
#include <EthernetClient.h>

#include "Adafruit_MQTT.h"                                  // Adafruit MQTT library 
#include "Adafruit_MQTT_Client.h"

#include <Stepper_28BYJ_48.h>                               // Stepper Motor library

/* =====================        Stepper Motor setup     =====================*/

Stepper_28BYJ_48 stepper(7,6,5,4);                          // Stepper Motor pins
long motorPosition = 0;                                     // Initial Motor position                 

const int STEPS_ONTHEWAY     = 0;                           // How many steps the motor needs to move 
const int STEPS_HOME         = 85;                          // to point to a specific position on the clock
const int STEPS_HOLIDAY      = 170;
const int STEPS_MORTALPERIL  = 256;
const int STEPS_GROCERY      = 341;
const int STEPS_UNIVERSITY   = 426;

const int buzzer = 9;                                       // Buzzer (Pin 9)


/* =====================       Ethernet & MQTT setup    =====================*/

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };        // Ethernet shield MAC addrees

#define AIO_SERVER      "io.adafruit.com"                   // Adafruit IO access
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "your_username"
#define AIO_KEY         "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

EthernetClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

#define halt(s) { Serial.println(F( s )); while(1);  }  
Adafruit_MQTT_Subscribe clockStatus = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/weasleyclockstatus");



void setup() {

    pinMode(buzzer, OUTPUT);                                // Set buzzer as an output
    
    Serial.begin(115200);                                   // Start the serial connection
    while(! Serial);                                        // Wait for serial monitor to open

    Serial.println("*********");                            // Say hello
    Serial.println("Oh, hello there,\nI'm Clockimo. Let's begin?\n");

    Serial.println("Esteblishing internet connection...");
    Ethernet.begin(mac);                                    // Start ethernet connection
    Serial.println("My IP address: ");                      // Print IP address
    Serial.println(Ethernet.localIP()); 
    delay(1000); 
    
    mqtt.subscribe(&clockStatus);                           // Connect to Adafruit IO
    Serial.println("*********\n\n\n");
}

uint32_t x=0;





void loop() {

    Serial.println("\nConnecting to feed...\n*");            
    MQTT_connect();                                         // Run the connecting void (below)

    Serial.println("Reading the feed...\n*\n*\n*\n*\n*");            
    Adafruit_MQTT_Subscribe *subscription;                  // Subscribe to feed
    
    while ((subscription = mqtt.readSubscription(1000))) {
        if (subscription == &clockStatus) {                 // Reading latest value
            Serial.print(F("\nLatest value: "));            // Converting array to string
            String fValue ((char *)clockStatus.lastread);
            Serial.print(fValue);
            Serial.println("\n");
            
            if(fValue == "id_home") {                       // if string match data from the feed
                Serial.println("Home");
                buzzSound();                                // play sound notification (see void below)
                stepBySteps(STEPS_HOME);                    // change stepper motor position (see void below)
            }
            if(fValue == "id_holiday") {
                Serial.println("on holiday");
                buzzSound();
                stepBySteps(STEPS_HOLIDAY);
            }
            if(fValue == "id_peril") {
                Serial.println("Mortal Peril");
                buzzSound();
                stepBySteps(STEPS_MORTALPERIL);
            }
            if(fValue == "id_grocery") {
                Serial.println("at Tesco");
                buzzSound();
                stepBySteps(STEPS_GROCERY);
            }
            if(fValue == "id_university") {
                Serial.println("at University");
                buzzSound();
                stepBySteps(STEPS_UNIVERSITY);
            }
            if(fValue == "id_ontheway") {
                Serial.println("at University");
                buzzSound();
                stepBySteps(STEPS_UNIVERSITY);
            }  
        }
    }
    
    Serial.print(F("Latest value hasn't changed.\n\n"));
    
    if(! mqtt.ping()) {                                     // Ping the server to keep the mqtt connection alive
        mqtt.disconnect();
    }
    Serial.println("\n\n\n");
}






void MQTT_connect() {
    
    int8_t ret;
    
    if (mqtt.connected()) {                                 // if connected then return to the loop void
        return;
    }

    Serial.print("Connecting to MQTT... ");
    while ((ret = mqtt.connect()) != 0) {                   // Connect (or retry to connect)
            Serial.println(mqtt.connectErrorString(ret));
            Serial.println("Retrying MQTT connection in 5 seconds...");
            mqtt.disconnect();
            delay(5000);
            Serial.println("\n\n\n");
    }
    
    Serial.println("Connected!\n");
}

void stepBySteps(int newPosition) {                         // Calculate how many steps motor should do
    if(motorPosition == newPosition) {
        Serial.println("No movement required.");
        return;
    }
    
    long steps = newPosition - motorPosition;
    
    stepper.step(steps);
    motorPosition = newPosition;
    Serial.print("position should now be:" );
    Serial.println(motorPosition);
}

void buzzSound() {                                          // Sound notification
    tone(buzzer, 1000); 
    delay(200);        
    noTone(buzzer);    
    delay(200);
    tone(buzzer, 1000); 
    delay(200);        
    noTone(buzzer);    
    delay(200);         
    tone(buzzer, 1000); 
    delay(200);        
    noTone(buzzer);                      
}
