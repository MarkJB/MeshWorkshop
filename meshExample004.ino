#include <ESP32Servo.h>
#include <analogWrite.h>
#include <ESP32Tone.h>
#include <ESP32PWM.h>



//************************************************************
// this extends the simple example and uses the painlessMesh library to send some data from a sensor and react to received values
//
// 1. Reads the sensor value and sends it to some nodes - what should this be? All nodes, one node, a group of nodes?
// 2. prints anything it receives to Serial.print and also output that value somehow
//
//************************************************************

// Thnigs we need to define to make that happen are:
// - Constrained input values (e.g. 0-255 for all sensors regardless of type)
// - Contrained output values (e.g. 0-255 for all output devices regardless of type)
// - A common node name and password (MESH_PREFIX & MESH_PASSWORD) as long as these match, we should be able to talk on the mesh
// - A common message format so all nodes and send and recive data in a way they can all act on - typically json (Javascript Object Notation)

#include "painlessMesh.h"  // Include the painlessMesh library so we can use it
//#include <Servo.h>  // Include the servo library so we can use it


#define   MESH_PREFIX     "whateverYouLike"  // The name of our mesh network
#define   MESH_PASSWORD   "somethingSneaky"  // The password to access our mesh network
#define   MESH_PORT       5555  // A network port that our mesh nodes will use to connect to and communicate on the network

#define   SERVO_PIN       2  // GPIO2  // Convenient pin near 5v and Gnd - might work - might not - needs testing
#define   TONE_PIN        4  // GPIO4  // Convenient pin near Gnd. Note: ESP8266 WeMos Clone GPIO0 doesn't boot with speaker attached
#define   ANALOG_SENSOR_PIN        36 // A0  // ADC0  // The only analog input on the ESP8266 (There are more on the ESP32)

Servo outputServo;  // An instance of the servo library, called 'outputServo' that we can use to control a servo (once we tell it how)

Scheduler userScheduler; // A task scheduler to control our actions (when and how often we read the sensors and send that data on the mesh)
painlessMesh  mesh;  // Create a 'mesh' object from the library (this gives us access to the library in our code)

// User stub (a stub is a definition or a minial implementation of a function - its details can be defined later)
void sendMessage();

// Do sendMessage() every second forever.
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

// Now we can say what we want sendMessage() to actually do...
// For the next example, we will generate a random number to represent our sensor data.
// As mentioned above, the value will be between 0 and 255. So we can generaete a random number in that range with random(min, max) (where min=0 and max=255)
void sendMessage() {
  // Read our analog input to get a value from our sensor (these are analog sensors)
  int analogInput = analogRead(ANALOG_SENSOR_PIN);

  // Uncomment one of the constrainedVal lines to match the sensor in use.
  // Temperature Sensor Module observed values
  //int constrainedVal = constrain(map(micInput, 500, 400, 0, 255),0,255);  // Map the range of observed values to our agreed upon range of 0-255
  // Microphone Sensor Module observed values
  int constrainedVal = constrain(map(analogInput, 800, 900, 0, 255),0,255);  // Map the range of observed values to our agreed upon range of 0-255
  Serial.printf("Analog read %u, mapped %u\n", analogInput, constrainedVal);  // Output the input and mapped/constrained values for testing and tuning
  
  mesh.sendBroadcast(String(constrainedVal));  // Next we broadcast the sensor value on our mesh - with any luck all nodes will get it.
  // Note: we could send a message to one node if we know its nodeId using mesh.sendSingle(dest, msg)
  // By default We send a new sensor value every second, but we could set a random delay before we send another message to make things interesting
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 1 ));  
}


// Define some call back functions - these are just functions that the library expects to exist, and they will be called when certain things happen

// Now that we have received a value from another node, we should set our output device to use that value there-by reacting to the input values
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Message received from %u msg=%s\n", from, msg.c_str());  // Print the received message to the serial console
  // Now we can set our output device to the input value. Lets say our output device is a servo.
  unsigned int val = msg.toInt();  // Convert the incoming message to the right type of int for tone()
  outputServo.write(val); // Now write the random value to the servo (Note: msg is a string and servo expects a number so we convert it with .toInt())
  tone(TONE_PIN, val);  // Write the received data to the tone library (play a tone based on the received data)
}

// What we do when a connection happens (receives the nodeId - presumably this is the id we are given when we join the mesh)
void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("New Connection, nodeId = %u\n", nodeId);  // Print the message (and our node id) to the serial console
}

// What we do when a connection changes (receives nothing)
void changedConnectionCallback() {
  Serial.printf("Changed connections\n");  // Print the message to the serial console - do we need to do anything else here? Probably not... the mesh should reconnect when able
}

// What we do when the time on the node is adjusted - no idea yet what this is for, but I suspect its used to maintain the node and work out node routing
void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);  // Print the message to the serial console
}

// Now we actually do some work... 
// This will do the following:
void setup() {
  Serial.begin(115200);  // setup our node to talk on the serial console
  pinMode(TONE_PIN, OUTPUT);  // State that we want to use this pin as an output
  pinMode(SERVO_PIN, OUTPUT);  // State that we want to use this pin as an ouput
  pinMode(ANALOG_SENSOR_PIN, INPUT);  // State that we want to use this pin as an input
  outputServo.attach(SERVO_PIN);  // Here we tell outputServo which pin to use
  // Setup the mesh and tell it what the call back functions it expects are called
  // mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskSendMessage ); // Create our scheduled user task and start it running (remember this is the thing that does our work)
  taskSendMessage.enable();  // Enable our user task (basically start it running)
}

// The main loop doesn't do much as all our work is done in the call backs and our scheduled task
void loop() {
  mesh.update();
}
