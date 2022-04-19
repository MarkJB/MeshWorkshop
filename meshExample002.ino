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

#define   MESH_PREFIX     "whateverYouLike"  // The name of our mesh network
#define   MESH_PASSWORD   "somethingSneaky"  // The password to access our mesh network
#define   MESH_PORT       5555  // A network port that our mesh nodes will use to connect to and communicate on the network


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
  String msg =  String(random(0,255));  // This is our dummy sensor data - its a number and message expects a string so we have to convert it with String()
  mesh.sendBroadcast( msg );  // Next we broadcast that message on our mesh - with any luck all nodes will get it.
  // Note: we could send a message to one node if we know its nodeId using mesh.sendSingle(dest, msg)
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 15 ));  // Then we set a random delay (between 1 and 15 seconds) before we send another message
}


// Define some call back functions - these are just functions that the library expects to exist, and they will be called when certain things happen

// What we do when we receive a message (This function is given the id of the sending node 'from' and the message the node sent 'msg' (not the same as the other 'msg' mentioned above))
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Message received from %u msg=%s\n", from, msg.c_str());  // Print the received message to the serial console
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
