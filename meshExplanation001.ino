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

// JSON is just (at a basic level) a list of key, value pairs: {key: value, key: value ... key: value}
// So can define our message as { data: 255 } and send that to some nodes on the mesh
// If we know that incoming messages have the same format, we can get that value by using the 'data' key. More on that later


#include "painlessMesh.h"  // Include the painlessMesh library so we can use it

#define   MESH_PREFIX     "whateverYouLike"  // The name of our mesh network
#define   MESH_PASSWORD   "somethingSneaky"  // The password to access our mesh network
#define   MESH_PORT       5555  // A network port that our mesh nodes will use to connect to and communicate on the network


Scheduler userScheduler; // A task scheduler to control our actions (when and how often we read the sensors and send that data on the mesh)
painlessMesh  mesh;  // Create a 'mesh' object from the library (this gives us access to the library in our code)

// User stub (a stub is a definition or a minial implementation of a function - its details can be defined later)
void sendMessage() ; // Prototype so PlatformIO doesn't complain

// Here we define a 'task' (we'll call it later to start it running)
// We are saying create a new 'Task' called 'taskSendMessage' that will do something every 1 seconds (TASK_SECOND is defined as 1000UL (unsigned long) which is basically 1000mSeconds)
// We say do that task forever (TASK_FOREVER is provided by the task scheduler library, but we could say only do this 1000 times if we wanted
// The thing it will do is call our 'sendMessage()' function. We still haven't said what that actually does, but by defining it as a stub earlier, we can use it here.
// tl;dr, do sendMessage() every second forever.
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

// Now we can say what we want sendMessage() to actually do...
void sendMessage() {
  String msg = "Hello from node ";  // Define a string called 'msg' that just says 'Hello from node '
  msg += mesh.getNodeId();  // Here we get our node id (by calling mesh.getNodeID()) and then append that into the end of our message
  // msg will now be something like 'Hello from node 12354' (no idea what that will be, but according to the docs, it returns a uint32 so could be a long number)
  mesh.sendBroadcast( msg );  // Next we broadcast that message on our mesh - with any luck all nodes will get it.
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 5 ));  // Then we set a random delay (between 1 and 5 seconds) before we send another message
}

// Define some call back functions - these are just functions that the library expects to exist, and they will be called when certain things happen

// What we do when we receive a message (This function is given the id of the sending node 'from' and the message the node sent 'msg' (not the same as the other 'msg' mentioned above))
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());  // Print the received message to the serial console
}

// What we do when a connection happens (receives the nodeId - presumably this is the id we are given when we join the mesh)
void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);  // Print the message (and our node id) to the serial console
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
// All we do here is update the mesh.
// It looks like this would run as fast as it can, but there is probably some scheduler running within mesh to keep things running smoothly
void loop() {
  mesh.update();
}
