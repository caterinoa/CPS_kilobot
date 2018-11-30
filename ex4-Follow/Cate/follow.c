#include <kilombo.h>
#include "follow.h"

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

/******* CONSTANTS ********************/
static const uint8_t MAX_INT = 255; 

/******* END OF GAME ********************/

void checkIfWinning(){
  if(mydata->sender_id == mydata->runner.runner_id){
      // the runner has been caught 
      set_color(GREEN);
    }else{
      // the kilobot in the danger area is not the runner
      set_color(RED);
    }
}

/******* DISTANCE *********************/

// Register only the closest bot distance
void updateMinDistance(distance_measurement_t *d){
	uint8_t new_dist = estimate_distance(d);
	if(mydata->sender_id == mydata->min_bot){	// The closest bot changed distance: update!
		mydata->min_distance = new_dist;
	}else{
		if(new_dist < mydata->min_distance){	// There's a new closest bot: update!
			mydata->min_distance = new_dist;
			mydata->min_bot = mydata->sender_id;
		}
	}
}

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

void get_random_direction(){
  uint8_t random = rand_soft() % 3; // 0 <= random < 3
  switch(random){
      case 0:
        set_motion(LEFT); break;
      case 1:
        set_motion(FORWARD); break;
      case 2:
        set_motion(RIGHT); break;
    }
}

void wait(uint8_t sec){
  uint8_t waitTime = sec*3200 + kilo_ticks;
  if(kilo_ticks < waitTime){
    wait(sec-1);
  }
}

/******** COLLISION AVOIDANCE ****************/

uint8_t collisionDetected(){
	if(mydata->min_distance <= WARNING_D){
		return 1;
	}else{
		return 0;
	}
}

uint8_t isInDanger(){
	if(mydata->min_distance < DANGER_D){
		return 1;
	}else{
		return 0;
	}
}

void avoidCollision(){
	if(isInDanger()){		// DANGER AREA --> STOP
		
    set_motion(STOP);
    mydata->stopped = 1; // stops the communication
    if(kilo_uid % 2 == 1) { // CATCHER
      checkIfWinning();
    } else {
      set_color(RED);
    }

	}
}


/******** RANDOM *******************************/

// initialize the seed of soft generator with a random number
void setupSeed(){
    rand_seed(rand_hard());
}



/******* MESSAGE EXCHANGING ******************/

// Generate a message
void setup_message(){
  mydata->transmit_msg.type = NORMAL;      // Message type
  uint8_t sender = kilo_uid;			     // Add ID on the first 4 bits (on the left)
  mydata->transmit_msg.data[0] = sender;
  mydata->transmit_msg.crc = message_crc(&mydata->transmit_msg);    // Checksum
}

// Callback to send a message
message_t *message_tx(){
  setup_message();      //Generate a new message every time!
  return &mydata->transmit_msg;
}

// Get the sender uid
void getSenderID(message_t m){
	mydata->sender_id = m.data[0];		// Raw message
}
  
void update_incoming_message(message_t *m, distance_measurement_t *d){
  mydata->message_arrived = 1;
  mydata->new_message = 1;
  getSenderID(*m);
  updateMinDistance(d);
}

void updateRunnerInfo(distance_measurement_t *d){
  if(mydata->sender_id == mydata->runner.runner_id){
      mydata->runner.in_range = 1;
      mydata->runner.new_distance = estimate_distance(d);
    } else {
      mydata->runner.in_range = 0;
      mydata->runner.new_distance = MAX_INT;
  }
}

// Callback to Receive messages (only for catchers)
void message_rx_catcher(message_t *m, distance_measurement_t *d) {
    if(mydata->stopped==0){
      update_incoming_message(m, d);
      updateRunnerInfo(d);
      printf("Bot %d -> | sender: %d|\n", kilo_uid, mydata->sender_id);
    }
}

// Callback to Receive messages (only for runners)
void message_rx_runner(message_t *m, distance_measurement_t *d) {
    if(mydata->stopped==0){
      update_incoming_message(m, d);
      printf("Bot %d -> | sender: %d |\n", kilo_uid, mydata->sender_id);
    } 
}
// Read a message (if available) and perform an action
void readMessage(){
  mydata->new_message = 0;            //Reset Flag
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
    set_color(OFF);
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


/******** RUN AND FOLLOW ****************/

uint8_t get_id_to_follow(){
  return kilo_uid-1;    // just for testing, the catchers follow the runner with id equal to its own id minus 1
  // in the project it will depend on the colour chosen by the witch
}

void changeDirection(){
  // changes direction in a cylcic way (FORWARD->RIGHT->LEFT)
  mydata->runner.last_direction = (mydata->runner.last_direction + 1) % 3;
}

void setDirection(){
  if(mydata->runner.last_distance < mydata->runner.new_distance) { // if the runner is getting far, the catcher changes direction
    changeDirection();
  }
  switch(mydata->runner.last_direction){
      case 0:
        set_motion(LEFT); break;
      case 1:
        set_motion(FORWARD); break;
      case 2:
        set_motion(RIGHT); break;
    }
}

void follow(){

  if(mydata->runner.in_range == 0){    // the runner is out of range
    get_random_direction();
  } else{                              // the runner is in range
    setDirection();
  }              

}


/******* SETUP,LOOP,MAIN *******************/

void loop() {
  if(mydata->new_message){  // Disperse Phase
    readMessage();
  }

  if(kilo_uid % 2 == 0){  // kilobots with even id are runners 
    get_random_direction();
  } else{                 // kilobots with odd id are catchers 
    follow();
  }

  if(collisionDetected()){
    avoidCollision();
    // If there danger area is reached with the runner, the catcher blinks GREEN
    // If there is a collision with another kilobot the catcher blinks RED
    // The runner blinks RED
  }

  wait(8); // wait 2 seconds to turn right or left

}


void setup(){
  // Random
  setupSeed();
  // Message variables
  setup_message(rand_soft()); 
  mydata->stopped = 0; //false
  mydata->new_message = 0;
  kilo_message_tx = message_tx;
  // Messages
  mydata->message_arrived = 0;
  // Time Management 
  resetClock(BLINK_C);
  resetClock(DEFAULT_C);
  // Distance
  mydata->min_distance = MAX_INT;
  mydata->min_distance = MAX_INT;
  mydata->min_bot = -1;
  // Color
  assignInitialColor();
  // Runner
  if(kilo_uid % 2 == 0){  // kilobots with even id are runners 
    kilo_message_rx = message_rx_runner;
    set_color(BLUE);
  } else{                 // kilobots with odd id are catchers 
    set_color(PURPLE);
    kilo_message_rx = message_rx_catcher;
    runner_t r = {.runner_id = -1, .last_distance = -1, .new_distance = -1, .last_direction = FORWARD, .in_range = 0}; // initialising the runner infos
    mydata->runner = r;
    mydata->runner.runner_id = get_id_to_follow(); // deciding the kilobot to follow
    printf("runner: %d\n", mydata->runner.runner_id);
  }
}


int main() {
  kilo_init();
  kilo_start(setup, loop);
  return 0;
}





