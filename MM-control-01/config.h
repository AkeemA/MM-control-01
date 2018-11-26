//! @file
//! @brief main configuration file

#ifndef CONFIG_H_
#define CONFIG_H_


// ###### BEGIN - WOJCIECH KOPROWSKI CONF:

#define EMULATED_LEDS
//#define SERIAL_BUTTONS

//#define DEBUG

#ifdef DEBUG
  #define DEBUG_MENU

  //#define BTN_LOG
  //#define BTN_TRACE
  //#define MAIN_LOG
  //#define MAIN_TRACE
  //#define MMCTL_LOG
  //#define MMCTL_TRACE
  //#define MOTION_LOG
  //#define MOTION_TRACE
  //#define PSTOR_LOG
  //#define PSTOR_TRACE
  //#define STEPPER_LOG
  //#define STEPPER_TRACE
#endif

//MOTOR AND STEPPER
//
// PULLEY MOTOR
// SETUP:
// - A4988: 1/16 microstepping
// - Chinese NEMA 17 (I guess: 200 steps/rev)
// - MK8 drive gear: from spec: Effective diameter: 6.7mm; Effective circumference : 21mm
// Steps for extrude 1 mm of filament:
// steps = (steps per revolution) * (microstepping) / (effective circumference)
// x = 200 * 16 / 21 ~= 152
#define PULLEY_STEPS_MM 152

// IDLER MOTOR
// SETUP:
// - A4988: 1/16 microstepping
// - Chinese NEMA 17 (I guess: 200 steps/rev)
// - Prusa selector -> one rotation from first bearing to the next
// Steps to change bearing:
// ~23 - taken from Chuck Kozlowski config: https://github.com/cskozlowski/mmu2/
#define IDLER_STEPS_TO_CHANGE 23
//
// SELECTOR MOTOR
// SETUP:
// - A4988: 1/16 microstepping
// - Chinese NEMA 17 with integrated lead screw (I guess: 200 steps/rev)
//   Lead screw: 8mm pitch; 8mm diameter
// Steps to move for 1 mm:
// steps = (steps per revolution) * (microstepping) / (pitch)
// x = 200 * 16 / 8 = 400
#define SELECTOR_STEPS_MM 400

// Ramps AUX-1 pinout:
// [5V] [GND]   [D1]     [D0]
// [5V] [GND] [A3/D57] [A4/D58]
// ------- BOARD EDGE ---------
//
// In AUX-1 you can connect 2 sensors even if they are "Digital" or "Analog"
#define findaPin 3 // Ramps AUX-1 pin A3
#define extruderSensorPin 4 // Ramps AUX-1 pin A4

// Uncoment to use unstock finda sensor (in my case 12mm inductive sensor).
//#define UNSTOCK_FINDA


#define DIR_PIN 0
#define STEP_PIN 1
#define ENABLE_PIN 2
#define SPEED_VALUE 3
// Ramps X-axis
#define selectorDirPin    55
#define selectorStepPin   54
#define selectorEnablePin 38
#define selectorSpeed     2000
const int selectorMotor[4] = { selectorDirPin, selectorStepPin, selectorEnablePin,  selectorSpeed };

// Ramps Y-axis
#define idlerDirPin       61
#define idlerStepPin      60
#define idlerEnablePin    56
#define idlerSpeed        2000
const int idlerMotor[4] = { idlerDirPin, idlerStepPin, idlerEnablePin, idlerSpeed };

// Ramps Z-axis
#define pulleyDirPin      48
#define pulleyStepPin     46
#define pulleyEnablePin   62
#define pulleySpeed       2000
const int pulleyMotor[4] = { pulleyDirPin, pulleyStepPin, pulleyEnablePin,  pulleySpeed };

// ###### END - WOJCIECH KOPROWSKI CONF

//UART0
#define UART0_BDR 115200

//UART1
#define UART1_BDR 115200

//communication uart0/1
#define UART_COM 1

//0 - PULLEY
//1 - SELECTOR
//2 - IDLER
#define AX_PUL 0
#define AX_SEL 1
#define AX_IDL 2

//signals (from interrupts to main loop)
#define SIG_ID_BTN             1 // any button changed

//states (<0 =error)
#define STA_INIT               0 //setup - initialization
#define STA_BUSY               1 //working
#define STA_READY              2 //ready - accepting commands

//number of extruders
#define EXTRUDERS 5

#endif //CONFIG_H_
