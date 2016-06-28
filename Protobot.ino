#include <Servo.h>
#include <PS2X_lib.h>

int dummy;                  // Dummy variable to work around IDE (1.0.3) #ifdefs pre-processor bug. 
//#define DEBUG             // Uncomment to turn on debugging output.
                            
// Specify pins for servos, motors, PS2 controller connections. 
#define PS2_CLK_PIN   9     // Clock
#define PS2_CMD_PIN   7     // Command
#define PS2_ATT_PIN   8     // Attention
#define PS2_DAT_PIN   6     // Data

#define ARM_SERVO_PIN 4     // Elbow Servo HS-755HB

#define VM_PIN        // Number goes here
#define SM_PIN        // Number goes here
#define GM_PIN        // Number goes here

// PS2 controller characteristics.
#define JS_ZERO       128     // Joystick midpoint value.
#define JS_RANGE      124
#define JS_DEAD       4       // Joystick deadzone value.
#define Z_STEP        2.0     // Change in z-axis (mm) per button press.

// Default servo speeds. For CRS, 0 = full speed in one direction, 90 = zero speed, 180 = full speed in opposite direction.
#define R_SPD_ZERO 90
#define R_SPD_RANGE 90
float R_SPD_SCALE = R_SPD_RANGE / JS_RANGE;
#define T_SPD_ZERO 90
#define Z_SPD_ZERO 90

// Global variables storing servo speeds. Initialize to default speeds.  
float r_spd = R_SPD_ZERO;
float t_spd = T_SPD_ZERO;
float z_spd = Z_SPD_ZERO;

// Declare PS2 controller and servo objects
PS2X  Ps2x;
Servo arm_servo;
 
void setup() {
#ifdef DEBUG
  Serial.begin(115200);
#endif
  arm_servo.attach(ARM_SERVO_PIN);
  pinMode(VM_PIN, OUTPUT);
  pinMode(GM_PIN, OUTPUT);
  pinMode(SM_PIN, OUTPUT);
  
  // Set up PS2 controller; loop until ready.
  byte ps2_status;
  do {
    ps2_status = Ps2x.config_gamepad(PS2_CLK_PIN, PS2_CMD_PIN, PS2_ATT_PIN, PS2_DAT_PIN);
#ifdef DEBUG
  if (ps2_status == 1) Serial.println("No controller found. Re-trying . . .");
#endif
  } while (ps2_status == 1);
#ifdef DEBUG
  switch (ps2_status) {
    case 0:
      Serial.println("Found Controller, configured successfully.");
      break;
    case 2:
      Serial.println("Controller found but not accepting commands.");
      break;
    case 3:
      Serial.println("Controller refusing to enter 'Pressures' mode, may not support it. ");      
      break;
  }
#endif
  // Park robot when ready.                            
  set_robot(R_SPD_ZERO, R_SPD_ZERO, R_SPD_ZERO);
#ifdef DEBUG
  Serial.println("Started.");
#endif
  delay(500);
}
 
void loop()
{
  // Indidates whether input can move arm. 
  boolean move_arm = false;

  Ps2x.read_gamepad();
  // Read right joystick; adjust value with respect to joystick zero point.
  float rh_js_y = (float)(Ps2x.Analog(PSS_RY) - JS_ZERO);
  if (abs(rh_js_y) > JS_DEAD) {
    r_spd = R_SPD_ZERO + rh_js_y * R_SPD_SCALE;
    move_arm = true;
  }
    
  // z
  if (Ps2x.Button(PSB_L1) || Ps2x.Button(PSB_R1)) {
    if (Ps2x.Button(PSB_L1)) {
      z_temp -= Z_STEP;
    } else {
      z_temp += Z_STEP;
    }
    // Must be positive.
    z_temp = max(z_temp, 0);
    move_arm = true;
  }
  
  // Check if motion is needed.
  if (move_arm) {
    if (set_robot(r_spd, t_spd, z_spd)) {
      // If the arm was positioned successfully, record the new vales. Otherwise, ignore them.
      T = t_temp;
      Z = z_temp;
    } else {
      Serial.print("IK_ERROR.");
    }
    // Reset the flag
    move_arm = false;
  }
  delay(10);
}
 
// Position robot.
int set_robot(float r_spd, float t_spd, float z_spd)
{
  arm_servo.write(r_spd);
  analogWrite(VM_PIN, speed);
  analogWrite(SM_PIN, speed);
  analogWrite(GM_PIN, speed);
  return true;
}
