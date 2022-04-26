//************************************************************
// this extends the simple example and uses the painlessMesh library to send some data from a sensor and react to received values, but this time the data is sent as json
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

#include <ArduinoJson.h>  // Library to create and decode json format messages
#include "painlessMesh.h"  // Include the painlessMesh library so we can use it
#include <Servo.h>  // Include the servo library so we can use it

#define   MESH_PREFIX     "whateverYouLike"  // The name of our mesh network
#define   MESH_PASSWORD   "somethingSneaky"  // The password to access our mesh network
#define   MESH_PORT       5555  // A network port that our mesh nodes will use to connect to and communicate on the network

#define   SERVO_PIN       2  // GPIO2  // Convenient pin near 5v and Gnd - might work - might not - needs testing
#define   TONE_PIN        4  // GPIO4  // Convenient pin near Gnd. Note: ESP8266 WeMos Clone GPIO0 doesn't boot with speaker attached
#define   ANALOG_SENSOR_PIN         A0  // ADC0  // The only analog input on the ESP8266 (There are more on the ESP32)

Servo outputServo;  // An instance of the servo library, called 'outputServo' that we can use to control a servo (once we tell it how)

Scheduler userScheduler; // A task scheduler to control our actions (when and how often we read the sensors and send that data on the mesh)
painlessMesh  mesh;  // Create a 'mesh' object from the library (this gives us access to the library in our code)

// User stub (a stub is a definition or a minial implementation of a function - its details can be defined later)
void sendMessage();

// Do sendMessage() every second forever.
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

// Now we are going to send our data as JSON
void sendMessage() {
  // Read our analog input to get a value from our sensor (these are analog sensors)
  int analogInput = analogRead(ANALOG_SENSOR_PIN);
 
  DynamicJsonDocument jsonDoc(1024);  // Create an instance of DynamicJsonDocument that is 1024 bytes and called jsonDoc

  // Set the type to match your sensor (you can have more than one)
  // Set "low" to the low range of the observed raw values
  // Set "high" to the high range of the observed raw values
  // Raw can just be the real value if using something like the DHT30 temperature sensor
  // Temperature Sensor Module observed values (COM8) Low 500, High 300 (This sensor works back to front. High is low, low is high)
  // Microphone Sensor Module observed values (COM13) Low 700, High 900

  jsonDoc["raw"] = analogInput;  // Add the raw sensor value to the json object under the key "raw"
  jsonDoc["type"] = "microphone";  // We can send the type of sensor so the receving node can choose how to react based on the type of sensor data it receives
  jsonDoc["low"] = 700;  // We can also send the upper and lower limts that sensor produces so we can choose how to interpret the data
  jsonDoc["high"] = 900;  
 
  // Output the json message to the serial console
  Serial.printf("Sending json string: ");
  serializeJson(jsonDoc, Serial);  
  Serial.printf("\n");

  // Convert the jsonDoc to a 'serial' format (basically a string) that can be sent over the network
  String msg;  // Create a msg variable of type string that is undefined
  serializeJson(jsonDoc, msg);  // Get a string representation of the JSON object and assign it to msg so we can send it via the mesh
  mesh.sendBroadcast(msg);  // Next we broadcast the JSON string on our mesh - with any luck all nodes will get it.
  
  // Note: we could send a message to one node if we know its nodeId using mesh.sendSingle(dest, msg)
  // By default We send a new sensor value every second, but we could set a random delay before we send another message to make things interesting
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 1 ));  
}


// Define some call back functions - these are just functions that the library expects to exist, and they will be called when certain things happen

// Now that we have received data from another node in JSON format, we have to do some conversion to get at the values
void receivedCallback( uint32_t from, String &msg ) {
  // Convert the serialized json received in 'msg' into a form that we can extract the data from
  DynamicJsonDocument jsonDoc(1024);  // Create an instance of DynamicJsonDocument that is 1024 bytes and called jsonDoc
  String jsonMsg = msg.c_str();  // Returns a pointer to an array that contains a null-terminated sequence of characters (i.e., a C-string) representing the current value of the string object.
  deserializeJson(jsonDoc, jsonMsg);

  // Print the received message to the serial console
  Serial.printf("Message received from nodeId=%u msg=", from);  
  serializeJson(jsonDoc, Serial);
  Serial.printf("\n");

  // Do something with the json data
  int constrainedVal = constrain(map(jsonDoc["raw"], jsonDoc["low"], jsonDoc["high"], 0, 255),0,255);  // Map and constrain the raw data using the limits to our agreed upon range of 0-255
  Serial.printf("Computed constrainedVal: %u\n", constrainedVal);
  
  // Now we can set our output device to the computed value. Lets say our output device is a servo.

  outputServo.write(constrainedVal); // Now write the value to the servo
  tone(TONE_PIN, constrainedVal);  // Write the received data to the tone library (play a tone based on the received data)
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
