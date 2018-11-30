
#define NO_MESSAGE 0    // Constant to declare no message content

// COLORS
#define OFF RGB(0,0,0)
#define RED RGB(3,0,0)
#define GREEN RGB(0,3,0)
#define BLUE RGB(0,0,3)
#define WHITE RGB(3,3,3)

// MOTION TYPE
typedef enum {
  STOP,
  FORWARD,
  LEFT,
  RIGHT
} motion_t;


// CLOCKS TYPES
typedef enum {
  BLINK_C,
  DEFAULT_C
} clock_type_t;


// GLOBAL VARIABLES
typedef struct 
{
  // Messages
  uint8_t new_message;     //Flag
  message_t transmit_msg;
  message_t received_msg;

  // Time Management
  uint8_t default_clock;
  uint8_t blink_clock;  // Used only to blink!
  // distance 
  uint8_t current_distance;
  distance_measurement_t dist;


  // State
  uint8_t message_arrived; //Flag
  uint8_t danger_flag; //Flag, eaquals to 1 when too close

} USERDATA;




