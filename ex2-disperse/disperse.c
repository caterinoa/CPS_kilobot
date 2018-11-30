#include <kilombo.h>
#include "disperse.h"

#ifdef SIMULATOR
#include <stdio.h> // for printf
#else
#include <avr/io.h>  // for microcontroller register defs
//  #define DEBUG          // for printf to serial port
//  #include "debug.h"
#endif

REGISTER_USERDATA(USERDATA)

/*
** 
*/

/******* MOVEMENT *********************/

void set_motion(motion_t new_motion)
{
  switch(new_motion) {
  case STOP:
    set_motors(0,0);
    break;
  case FORWARD:
    set_motors(kilo_turn_left, kilo_turn_right);
    break;
  case LEFT:
    set_motors(kilo_turn_left, 0); 
    break;
  case RIGHT:
    set_motors(0, kilo_turn_right); 
    break;
  }
}


/******** RANDOM *******************************/

// initialize the seed of soft generator with a random number
void setupSeed(){
    rand_seed(rand_hard());
}



/******* MESSAGE EXCHANGING ******************/

// Generate a message
void setup_message(uint8_t new_mex){
  mydata->transmit_msg.type = NORMAL;                               // Message type
  mydata->transmit_msg.data[0] = new_mex;                           // Payload
  mydata->transmit_msg.crc = message_crc(&mydata->transmit_msg);    // Checksum
}

// Callback to send a message
message_t *message_tx(){
  setup_message(rand_soft());       //Generate a new message every time!
  return &mydata->transmit_msg;
}

// Callback to Receive messages
void message_rx(message_t *m, distance_measurement_t *d) {
    mydata->message_arrived = 1;
    mydata->new_message = 1;
    mydata->received_msg = *m;
}


/******** PERFORM ACTION (Triggered by new mex) **********************/

motion_t getDirection(uint8_t mex){
  if(mex>0 && mex<64){           // 25% LEFT
    return LEFT;
  }else if(mex>=64 && mex<192){   // 50% FORWARD
    return FORWARD;
  }else if(mex>=192 && mex<256){  // 25% RIGHT
    return RIGHT;
  }else{                          // Error: off
    return STOP;
  }
}
  


// Perform an action reacting to a non-void message receive
void performAction(){
  uint8_t mex = mydata->received_msg.data[0];   // Just for code clearness
  // Change direction based on the message received
  switch(getDirection(mex)){
    case LEFT:         
      set_motion(LEFT);
      set_color(RED);
      break;
    case FORWARD:      
      set_motion(FORWARD);
      set_color(GREEN);
      break;
    case RIGHT:     
      set_motion(RIGHT);
      set_color(BLUE);
      break;
    default:                      
      set_motion(STOP);
      set_color(OFF);
  }
}

// Read a message (if available) and perform an action
void readMessage(){
  mydata->new_message = 0;            //Reset Flag
  if(mydata->received_msg.data[0]){   //If message not void
    performAction();                  // Perform action
  }
}


/******** TIME MANAGEMENT *******************/
uint8_t isInRange(uint8_t bottom, uint8_t top, clock_type_t c_type){
  // Calculate elapsed time 
  uint8_t elapsed_time;
  switch(c_type){
    case BLINK_C: elapsed_time = kilo_ticks - mydata->blink_clock; break;
    default: elapsed_time = kilo_ticks - mydata->default_clock;
  }
  // Check range
  if(elapsed_time >= bottom && elapsed_time < top){
    return 1; //True
  } else {
    return 0; //False
  }
}

void resetClock(clock_type_t c_type){
  switch(c_type){
    case BLINK_C: mydata->blink_clock = kilo_ticks; break;
    default: mydata->default_clock = kilo_ticks;
  }
}


/******* COMMANDS **************************/

void blink(uint8_t off_delay, uint8_t on_delay, uint8_t rgb_color){
  if(isInRange(0, off_delay, BLINK_C)){      // PHASE OFF
    set_color(RGB(0,0,0));
  } else if (isInRange(off_delay, off_delay+on_delay, BLINK_C)){  // PHASE ON
    set_color(rgb_color);
  } else {
    resetClock(BLINK_C); // Reset clock
  }
}

// Moves for a given amount of time (ticks). 
// Returns true when the movement is finished, otherwise false.
uint8_t move(motion_t direction, uint8_t duration){
  if(isInRange(0,duration,DEFAULT_C)){
    set_motion(direction);
    return 0;                 // Still moving, return false
  } else {
    set_motion(STOP);         // Movement ended, return true
    return 1;
  }
}

// Moves in a circle (more or less)
// Times are measured in ticks
void moveInCircle(uint8_t forward_time, uint8_t rotation_time, motion_t direction){
  if(isInRange(0,forward_time, DEFAULT_C)){                      
    set_motion(FORWARD);
    return;
  } else if(isInRange(forward_time, forward_time+rotation_time, DEFAULT_C)) {
    set_motion(direction);
    return;
  } else {
    resetClock(DEFAULT_C);
  }
}



/******* Utility **************************/

// Assign initial color
void assignInitialColor(){
  set_color(RGB(0,0,0));
}


// Check stable state:
// If i don't receive any message in a_time i'm alone
// NB: uses DEFAULT CLOCK
uint8_t checkIfAlone(uint8_t a_time){
  if(mydata->message_arrived){      // Reset
    resetClock(DEFAULT_C);
    mydata->message_arrived = 0;
    return 0;
  }else{
    if(isInRange(0, a_time, DEFAULT_C)){   // If is in range, not enough time elapsed
      return 0;
    }else{
      return 1;
    }
  }
}



/******* SETUP,LOOP,MAIN *******************/

void loop() {
  if(mydata->new_message){  // Disperse Phase
    readMessage();
  }

  if(checkIfAlone(64)){       // Stay Phase (192 = 6 sec)
    blink(32,64,WHITE);
    set_motion(STOP);
  }
}


void setup(){
  // Random
  setupSeed();
  // Message variables
  setup_message(rand_soft()); 
  mydata->new_message = 0;
  kilo_message_tx = message_tx;
  kilo_message_rx = message_rx;
  // States
  mydata->message_arrived = 0;
  // Time Management 
  resetClock(BLINK_C);
  resetClock(DEFAULT_C);
  // Color
  assignInitialColor();
}


int main() {
  kilo_init();
  kilo_start(setup, loop);
  return 0;
}





