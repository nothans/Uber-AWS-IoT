/*
 * Uber + AWS IoT + Lambda + Arduino
 * 
 * Use an AWS IoT connected Arduino Yun to indicate how far your Uber car
 * is away from you. This project started with BasicPubSub example by Amazon 
 * to interface with AWS IoT / MQTT.
 * 
 * https://www.hackster.io/nothans/use-uber-with-aws-iot-lambda-arduino-starter-kit-f71c50
 * 
 * Created: January 31, 2016 by Hans Scharler - http://www.nothans.com
 */

// Include AWS IoT
#include <aws_iot_mqtt.h>
#include <aws_iot_version.h>
#include "aws_iot_config.h"

// Setup AWS IoT Client
aws_iot_mqtt_client myClient; // init iot_mqtt_client

// Setup global variables for AWS IoT
char msg[32]; // read-write buffer
int cnt = 0; // loop counts
int rc = -100; // return value placeholder
bool success_connect = false; // whether it is connected

// Create a callback for AWS IoT messages
void msg_callback(char* src, int len) {
  
  Serial.println("CALLBACK:");  
  int i;
  for(i = 0; i < len; i++) {
    Serial.print(src[i]);
  }
  Serial.println("");

  // Get number of beeps
  int n = atoi(src);
  
  // Signal Uber's ETA by beeping the buzzer
  soundBeeps(n, 500, 300);  
  
}

// Define pins for button and buzzer
const int pinButton = 3;
const int pinBuzzer = 2;

void setup() {
  // Configure button pin as a digital input
  pinMode(pinButton, INPUT);

  // Configure buzzer pin as a digital output
  pinMode(pinBuzzer, OUTPUT);
    
  // Start serial for debug / monitoring
  Serial.begin(115200);

  // Wait for serial before AWS IoT setup starts
  while(!Serial);
  //
  char curr_version[80];
  sprintf(curr_version, "AWS IoT SDK Version(dev) %d.%d.%d-%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);
  Serial.println(curr_version);
  // Set up the client
  if((rc = myClient.setup(AWS_IOT_CLIENT_ID)) == 0) {
    // Load user configuration
    if((rc = myClient.config(AWS_IOT_MQTT_HOST, AWS_IOT_MQTT_PORT, AWS_IOT_ROOT_CA_PATH, AWS_IOT_PRIVATE_KEY_PATH, AWS_IOT_CERTIFICATE_PATH)) == 0) {
      // Use default connect: 60 sec for keepalive
      if((rc = myClient.connect()) == 0) {
        success_connect = true;
        // Subscribe to "topic1"
        if((rc = myClient.subscribe("uber_response", 1, msg_callback)) != 0) {
          Serial.println("Subscribe failed!");
          Serial.println(rc);
        }
      }
      else {
        Serial.println("Connect failed!");
        Serial.println(rc);
      }
    }
    else {
      Serial.println("Config failed!");
      Serial.println(rc);
    }
  }
  else {
    Serial.println("Setup failed!");
    Serial.println(rc);
  }
  // Delay to make sure SUBACK is received, delay time could vary according to the server
  delay(2000);  
}

void loop() {

  // Check if button is pressed
  if (digitalRead(pinButton)) {
    publishToAWSIoT();
  }

  if (success_connect) {
 
    // Get a chance to run a callback
    if ((rc = myClient.yield()) != 0) {
      Serial.println("Yield failed!");
      Serial.println(rc);
    }
        
  }
    
  delay(1000);
  
}

int publishToAWSIoT() {
  
  if (success_connect) {
    
    // Send the message "estimate" to the "uber_request" topic
    sprintf(msg, "{\"command\":\"estimate\"}", cnt);
    
    if ((rc = myClient.publish("uber", msg, strlen(msg), 1, false)) != 0) {
      Serial.println("Publish failed!");
      Serial.println(rc);
    }
    
    delay(1000);
    
  }
  
}

void soundBeeps(int beeps, int duration, int wait) {
  
    for (int i = 0; i < beeps; i++) {
      
        digitalWrite(pinBuzzer, HIGH);
        delay(duration);
        digitalWrite(pinBuzzer, LOW);
        delay(wait);
        
    }
    
}
