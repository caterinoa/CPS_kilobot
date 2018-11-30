// Constant to declare no message content
#define NO_MESSAGE 0    

// COLORS
#define OFF RGB(0,0,0)
#define RED RGB(3,0,0)
#define GREEN RGB(0,3,0)
#define BLUE RGB(0,0,3)
#define WHITE RGB(3,3,3)
#define PURPLE RGB(1,0,1)
#define YELLOW RGB(2,2,0)
#define ORANGE RGB(2,1,0)

// DISTANCE
#define DANGER_D 45
#define WARNING_D 60
#define SAFE_D 70

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
  uint8_t received_msg;
  uint8_t sender_id;

  // Distance
  uint8_t min_distance;
  uint8_t min_bot;

  // Time Management
  uint8_t default_clock;
  uint8_t blink_clock;  // Used only to blink!

  // State
  uint8_t message_arrived; //Flag

} USERDATA;




