// *** SentandReceive ***

// This example expands the previous Receive example. The Arduino will now send back a status.
// It adds a demonstration of how to:
// - Handle received commands that do not have a function attached
// - Send a command with a parameter to the PC

#include <CmdMessenger.h>  // CmdMessenger
#include "Registers.h"
#include "Motors.h"


// Attach a new CmdMessenger object to the default Serial port
CmdMessenger cmdMessenger = CmdMessenger(Serial3);

// This is the list of recognized commands. These can be commands that can either be sent or received. 
// In order to receive, attach a callback function to these events
enum
{
  kCommandList, // Command to request list of available commands
  kAcknowledge, // Command to acknowledge a received command
  kError, // Command to message that an error has occurred
  
  kPing,
  
  kStatus,
  kListEvents,
  kEvent,
  
  kListRegisters,
  kGetRegister,
  kGetRegisters,
  kSetRegister,
  kSetRegisters,
  
  kSleep,
  kWake,
  
  kLightingOn,
  kLightingOff,
  kSetLightingState,
  kLightingR,
  kLightingG,
  kLightingB,
  kLightingColor,
  kLightingBlend,
  
  kSetMotorSteps,
  kGetMotorSteps,
  kSetThetaMotorSteps,
  kGetThetaMotorSteps,
  kSetRadiusMotorSteps,
  kGetRadiusMotorSteps,
};

void ShowCommands() 
{
  Serial.println("Available commands");
  Serial.println(" 0;                 - This command list");
  Serial.println(" 3;                 - Ping the device");
  Serial.println(" 4;                 - Current status"); 
  Serial.println(" 5;                 - List all of the event IDs");
  Serial.println(" 6,<EventID>[,<extra>];                 - Send Event with and optional byte of data");
  Serial.println(" 7;                 - List all of the avaiable registers.");
  Serial.println(" 8,<register>;                 - Get the value of the given register");
  Serial.println(" 9,<register>,<count>;                 - Get <count> values starting from the given registers and incromenting up.");
  Serial.println(" 10,<register>,<value>;                 - Set the value of the given register");
  Serial.println(" 11,<register>,<count>,<value>[,<value> ...];                 - Set <count> values starting from the given registers and incromenting up.");
  Serial.println(" 12;                 - Put the system to sleep.");
  Serial.println(" 13;                 - Wake the system up.");
  Serial.println(" 14;                 - Lighting on.");
  Serial.println(" 15;                 - Lighting off.");
  Serial.println(" 16,<state>;                 - Set lighting state: 0 - Off, 1 - On");
  Serial.println(" 17,<value>;                 - Set value of red channel: 0x00 to 0xFF");
  Serial.println(" 18,<value>;                 - Set value of green channel: 0x00 to 0xFF");
  Serial.println(" 19,<value>;                 - Set value of blue channel: 0x00 to 0xFF");
  Serial.println(" 20,<value>,<value>,<value>;                 - Set colour of the lighting. 0x000000 to 0xFFFFFF");
  Serial.println(" 21,<value>;                 - Set time between lighting transistions");
  //TODO: Finish here this...
  //TODO: Flash storage for strings.
}






SystemID LookupMotor(char id){
  if(id == "T" || id == "t"){
    return MOTOR_THETA;
  }
  else if(id == "R" || id == "r"){
    return MOTOR_RADIUS;
  }
  return 0;
}
SystemID LookupMotor(char *id){
  return LookupMotor(id[0]);
}
SystemID LookupMotor(byte id){
  if(id == 0){
    return MOTOR_THETA;
  } 
  else if(id == 1){
    return MOTOR_RADIUS;    
  }
  return LookupMotor((char) id);
}



// Called when a received command has no attached function
void OnUnknownCommand()
{
  /* Args: void; Return: kStatus */
  cmdMessenger.sendCmd(kStatus, "Command without attached callback");
}
void OnPing()
{
  /* Args: void; Return: kAcknowledge "Pong" */
  cmdMessenger.sendCmd(kAcknowledge, "Pong");
}
void OnStatus()
{
  /* Args: void; Return: kStatus */
  char* remote_status = cmdMessenger.readStringArg();
  if(!remote_status){
    // TODO: Get status
    cmdMessenger.sendCmd(kStatus, "Okay"); 
  }
}

void OnEvent()
{
  /* Args: EventID event_id, byte extra; Return: kAcknowledge */
  EventID event_id = (EventID) cmdMessenger.readInt32Arg();
  byte extra = (byte) cmdMessenger.readCharArg();  
  evtManager.trigger(event_id, extra);
  cmdMessenger.sendCmd(kAcknowledge);
}

void OnSetRegister()
{
  /* Args: char register_address, byte value; Return: kAcknowledge */
  /* Args: char register_address, char value; Return: kAcknowledge */
  byte register_address = (byte) cmdMessenger.readCharArg();
  byte value = (byte) cmdMessenger.readCharArg();
  registers[register_address] = value;
  cmdMessenger.sendCmd(kAcknowledge);
}
void OnGetRegister()
{
  /* Args: char register_address, byte value; Return: kAcknowledge: byte value */
  byte register_address = (byte) cmdMessenger.readCharArg();
  byte value = registers[register_address];
  
  cmdMessenger.sendCmdStart(kAcknowledge);  
  cmdMessenger.sendCmdBinArg((byte) value);
  cmdMessenger.sendCmdEnd();
}
void OnSetRegisters()
{
  /* Args: char register_address, char register_count, byte values ... ; Return: kAcknowledge */
  byte register_address = cmdMessenger.readBinArg<byte>();
  byte register_count = cmdMessenger.readBinArg<byte>();

  for(unsigned int register_address = 0; register_address < register_count; ++register_address){
    byte value = cmdMessenger.readBinArg<byte>();

    // TODO: Overflow protection
    registers[register_address++] = value;    
  }
  cmdMessenger.sendCmd(kAcknowledge);
}
void OnGetRegisters()
{
  /* Args: char register_address, char register_count, byte values ... ; Return: kAcknowledge, byte value... */
  byte register_address = cmdMessenger.readBinArg<byte>();
  byte register_count = cmdMessenger.readBinArg<byte>();

  cmdMessenger.sendCmdStart(kAcknowledge);
  for(unsigned int register_address = 0; register_address < register_count; ++register_address){
    byte value = registers[register_address++];
    cmdMessenger.sendCmdBinArg((byte) value);
  }  
  cmdMessenger.sendCmdEnd();
}
void OnSleep()
{
  evtManager.trigger(SYSTEM_SLEEP_ACTION);
  cmdMessenger.sendCmd(kAcknowledge);
}
void OnWake(){
  // TODO: Can we even do this?
}


void OnLightingOn()
{
  evtManager.trigger(LIGHTING_TURN_ON);
  cmdMessenger.sendCmd(kAcknowledge);
}
void OnLightingOff()
{
  evtManager.trigger(LIGHTING_TURN_OFF);
  cmdMessenger.sendCmd(kAcknowledge);
}
void OnSetLightingState()
{
  bool turn_on = cmdMessenger.readBinArg<bool>();
  evtManager.trigger(LIGHTING_SET_STATE, turn_on);
  cmdMessenger.sendCmd(kAcknowledge);
}
void OnLightingR()
{
  byte color = cmdMessenger.readBinArg<byte>();
  evtManager.trigger(LIGHTING_SET_RED, color);
  cmdMessenger.sendCmd(kAcknowledge);
}
void OnLightingG()
{
  byte color = cmdMessenger.readBinArg<byte>();
  evtManager.trigger(LIGHTING_SET_GREEN, color);
  cmdMessenger.sendCmd(kAcknowledge);
}
void OnLightingB()
{
  byte color = cmdMessenger.readBinArg<byte>();
  evtManager.trigger(LIGHTING_SET_BLUE, color);
  cmdMessenger.sendCmd(kAcknowledge);
}
void OnLightingColor()
{
  byte rgb[3] = {
    cmdMessenger.readBinArg<byte>(),
    cmdMessenger.readBinArg<byte>(),
    cmdMessenger.readBinArg<byte>()
  };
  evtManager.trigger(LIGHTING_SET_COLOUR, rgb);
  cmdMessenger.sendCmd(kAcknowledge);
}
void OnLightingBlend()
{
  int blend_time = cmdMessenger.readBinArg<int>();
  evtManager.trigger(LIGHTING_SET_BLEND_TIME, blend_time);
  cmdMessenger.sendCmd(kAcknowledge);
}


void OnSetMotorSteps(SystemID motor_id){
  long new_position = cmdMessenger.readInt32Arg();
  evtManager.trigger(motor_id | POSITION, new_position);
  cmdMessenger.sendCmd(kAcknowledge);
}
void OnSetMotorSteps()
{
  SystemID motor_id = LookupMotor(cmdMessenger.readBinArg<byte>());
  OnSetMotorSteps(motor_id);
}


void OnGetMotorSteps(SystemID motor_id)
{
  long current_position;
  if(motor_id | MOTOR_THETA){
    current_position = get_theta_motor_steps();
  }
  else if(motor_id | MOTOR_RADIUS){
    current_position = get_radius_motor_steps();    
  }
  else{
    cmdMessenger.sendCmd(kError, "Unknown motor id.");
    return;
  }
  cmdMessenger.sendCmd(kAcknowledge, current_position);
}
void OnGetMotorSteps()
{
  SystemID motor_id = LookupMotor(cmdMessenger.readBinArg<byte>());
  OnGetMotorSteps(motor_id);
}
void OnSetThetaMotorSteps()
{
  OnSetMotorSteps(MOTOR_THETA);
}
void OnGetThetaMotorSteps()
{
  OnGetMotorSteps(MOTOR_THETA);
}
void OnSetRadiusMotorSteps()
{
  OnSetMotorSteps(MOTOR_RADIUS);
}
void OnGetRadiusMotorSteps()
{
  OnGetMotorSteps(MOTOR_RADIUS);
}


// Callbacks define on which received commands we take action
void attachCommandCallbacks()
{
  // Attach callback methods
  cmdMessenger.attach(OnUnknownCommand);
  cmdMessenger.attach(kCommandList, ShowCommands);
  cmdMessenger.attach(kPing, OnPing);
  cmdMessenger.attach(kStatus, OnStatus);
  cmdMessenger.attach(kEvent, OnEvent);
  cmdMessenger.attach(kSetRegister, OnSetRegister);
  cmdMessenger.attach(kGetRegister, OnGetRegister);
  cmdMessenger.attach(kSetRegisters, OnSetRegisters);
  cmdMessenger.attach(kGetRegisters, OnGetRegisters);
  cmdMessenger.attach(kSleep, OnSleep);
  cmdMessenger.attach(kWake, OnWake);
  cmdMessenger.attach(kLightingOn, OnLightingOn);
  cmdMessenger.attach(kLightingOff, OnLightingOff);
  cmdMessenger.attach(kSetLightingState, OnSetLightingState);
  cmdMessenger.attach(kLightingR, OnLightingR);
  cmdMessenger.attach(kLightingG, OnLightingG);
  cmdMessenger.attach(kLightingB, OnLightingB);
  cmdMessenger.attach(kLightingColor, OnLightingColor);
  cmdMessenger.attach(kLightingBlend, OnLightingBlend);
  cmdMessenger.attach(kSetMotorSteps, OnSetMotorSteps);
  cmdMessenger.attach(kGetMotorSteps, OnGetMotorSteps);
  cmdMessenger.attach(kSetThetaMotorSteps, OnSetThetaMotorSteps);
  cmdMessenger.attach(kGetThetaMotorSteps, OnGetThetaMotorSteps);
  cmdMessenger.attach(kSetRadiusMotorSteps, OnSetRadiusMotorSteps);
  cmdMessenger.attach(kGetRadiusMotorSteps, OnGetRadiusMotorSteps);
}


// Setup function
void command_setup() 
{
  // Listen on serial connection for messages from the PC
  Serial3.begin(115200); 

  // Adds newline to every command
  cmdMessenger.printLfCr();   

  // Attach my application's user-defined callback methods
  attachCommandCallbacks();

  // Send the status to the PC that says the Arduino has booted
  // Note that this is a good debug function: it will let you also know 
  // if your program had a bug and the arduino restarted  
  cmdMessenger.sendCmd(kStatus,"Arduino has started!");
}

// Loop function
void command_loop() 
{
  // Process incoming serial data, and perform callbacks
  cmdMessenger.feedinSerialData();
}


