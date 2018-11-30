
#define NO_MESSAGE 0    // Constant to declare no message content

// COLORS
#define RED RGB(3,0,0)
#define GREEN RGB(0,3,0)
#define BLUE RGB(0,0,3)
#define YELLOW RGB(0,3,3)

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


// STATES
typedef enum {
  MOVE_ONE,
  MOVE_TWO,
  MOVE_THREE
} move_state_t;


// GLOBAL VARIABLES
typedef struct 
{
  // Messages
  uint8_t new_message;     //Flag
  message_t transmit_msg;
  message_t received_msg;

  // Color
  uint8_t my_color;

  // State flag
  move_state_t current_state;

  // Time Management
  uint8_t default_clock;
  uint8_t blink_clock;  // Used only to blink!

} USERDATA;




