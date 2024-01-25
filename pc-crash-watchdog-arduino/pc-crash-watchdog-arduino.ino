/*
  Description: A PC Freeze/Crash Hardware Watch
      
  Abstract:
      Uses serial communication to establish "health" of the PC (system), when serial communication stops a 
      reset is pressed by sending a HIGH signal over the optocopler that connects directly to system RESET signal.

      LEDS:
      GREEN: Three blinks on startup
             Two short blinks before RESET signal followed by RED LED
             Two blinks before board self reset
             One blink for "ALIVE" ping message.
      RED:   Long blink before RESET signal      
      
  
  Author: Milan Stubljar of stubljar d.o.o. (milan@stubljar.com)
  
  Updated:  2022-01-07 HBTRIGGERDELAY changed from 60 to 120 sec
            2021-11-26 Refactored c:\src\PycharmProjects\Arduino\Milan-Stubljar\Monitor\pc-crash-watchdog\pc-crash-watchdog.inosoftwareReset to use PIN 4 and RST to trigger RESET 
                      Added multiblink at startup and before reset
            2021-11-24 Added code for NANO
  Created:  2018-08-09 Initial Version
  

  Change Log:
    2021-11-24 milan Added #define BOARD_ARDUINO and BOARD_MICROVIEW to add support for ARDUINO only boards
                     Added ARMED LED on PIN4 to indicate the ARMED STATE
    2020-02-11 milan
      Reset is now also triggered when unarmed, with larger delay: HB_UNARMED_TRIGGERDELAY = 1800 cylces.
    2018-08-09 milan 
      Initial code

  Notes:
    Disconet RST Pin before sketch upload!!!

*/

/* 
   #pragma GCC diagnostic warning "-fpermissive"
   #pragma GCC diagnostic ignored "-fpermissive"
*/

/*
 * use #define BOARD_ARDUINO for ARDUINO BOARDS
 * or  #define BOARD_MICROVIEW for MicroView - Oled Ardu8ino Module - https://www.sparkfun.com/products/12923
 */
#define BOARD_ARDUINO 1
/* #define BOARD_MICROVIEW 1 */ 

#ifdef BOARD_MICROVIEW
#include <MicroView.h>
#endif

#define HB 2        // declare LED pin 2 - RECEIVED heartbeat over serial - GREEN LED
#define OC 3        // declare LED pin 3 - TRIGGER OPTOCOUPLER + RED LED, connected to optocoupler
#define RP 4        // declare Reset PIN - Connected to RST pin on ARDUINO NANO BOARD

int cntHB = 0;
int HBTRIGGERDELAY = 120;           // max wait before triggering OC - RESET
int HB_UNARMED_TRIGGERDELAY = 1800; // max wait before triggering OC - RESET while not armed (must be big enough to allow for update)
// Note a loop is consisted of 1000+300ms delay + serial port read delay - on broken connection a serial time our is 1000
// When unarmed, that is 1000+250+1000ms = 2250ms for one loop (cnt++)

int cnt = 0;
int armed = 0;
int force = 0;
char terminator = '.';
String received = "";

String hbAliveMsg = "I am alive";
String hbRebootMsg = "Simply Reboot";

char buf[30];

int lcdMaxX, lcdMaxY, fontWidth, fontHeight;
int hbX, hbY;

int maxInt = 32767; // https://www.arduino.cc/reference/en/language/variables/data-types/int/


#ifdef BOARD_MICROVIEW
MicroViewWidget *hbWidget;  // declare widget pointer
#endif

void multiBlink(int blinkCnt, int delayOn, int delayOff);
void multiBlink(int blinkCnt);

void multiBlink(int blinkCnt, int delayOn, int delayOff) {
  for (int i = 0; i<blinkCnt; i++) {
    digitalWrite(HB, HIGH);
    delay(delayOn);
    digitalWrite(HB, LOW);
    delay(delayOff);
  }
} // multiBlink

void multiBlink(int blinkCnt) {
  multiBlink(blinkCnt, 350, 150);
}

// the setup routine runs once when you press reset:
void setup() {
  pinMode(RP, INPUT_PULLUP);      // set RESET pin as OUTPUT
  digitalWrite(RP, HIGH);
  delay(200);
  pinMode(RP, OUTPUT);      // set RESET pin as OUTPUT

#ifdef BOARD_MICROVIEW
  uView.begin();
  uView.clear(PAGE);
  lcdMaxX = uView.getLCDWidth();
  lcdMaxY = uView.getLCDHeight();
  fontHeight = uView.getFontHeight();
  fontWidth = uView.getFontWidth();
  hbX = fontWidth;
  hbY = lcdMaxY - fontHeight;

  hbWidget = new MicroViewSlider(0, 0, 0, HBTRIGGERDELAY); // declare RED widget as slider
#endif

  pinMode(OC, OUTPUT);      // set RED LED pin as OUTPUT
  digitalWrite(OC, LOW);

  pinMode(HB, OUTPUT);      // set GREEN LED pin as OUTPUT
  digitalWrite(HB, LOW);
 
  Serial.setTimeout(1000);  // sets the maximum milliseconds to wait for serial data. It defaults to 1000 milliseconds.
  Serial.begin(38400);      // open the serial port at 9600 bps:

#ifdef BOARD_MICROVIEW
    uView.setCursor(0,hbY);
    uView.print("hb: 0");
    uView.display();
#endif

  sprintf(buf, "DEBUG: Starting...");
    Serial.println(buf);
  multiBlink(3);            // Blink 3 times to indicate initialisation of the program
  delay(250);
  multiBlink(2);            // Blink 3 times to indicate initialisation of the program
  delay(250);
  multiBlink(1);            // Blink 3 times to indicate initialisation of the program
  
  sprintf(buf, "DEBUG: Blinks completed.");
    Serial.println(buf);
}

void softwareReset() {      // Restarts program from beginning by sending RESET SIGNAL from pin RP to RESET PIN
  // asm volatile ("  jmp 0");  
  multiBlink(2);
  delay(250);
  digitalWrite(RP, LOW);
} 

void processReceived() {
  if (received == hbRebootMsg) {
    armed = 1;
    force = 1;
  } else if (received == hbAliveMsg) {
    armed = 1;
    cnt = 0;
    cntHB++; // Increase number of received heartbeats
    if (cntHB == maxInt) {
      cntHB = 1;
    }
    digitalWrite(HB, HIGH);

#ifdef BOARD_MICROVIEW
    uView.circleFill(32, 24, 5, WHITE, NORM);
    hbWidget->setValue(cnt);
    // Print total number of heartbeats since last reset or power on
    uView.setCursor(0,hbY);
    uView.print(buf);
    uView.display();
    sprintf(buf, "DEBUG: hbX,Y: %d,%d", hbX, hbY);
      Serial.println(buf);
#endif

    sprintf(buf, "DEBUG: cntHB: %d", cntHB);
      Serial.println(buf);
    delay(300);               // wait for a second or two
    digitalWrite(HB, LOW);
  } else {
    cnt++;
    if (cnt == maxInt) {
      cnt == 1;
    }
    
#ifdef BOARD_MICROVIEW
    uView.circleFill(32, 24, 10, WHITE, NORM);
    hbWidget->setValue(cnt);
    uView.display();
#endif    
    delay(300);               // wait for a second or two
  }
} /* void processReceived() */ 

// the loop routine runs over and over again forever:
void loop() {
  // Attempt to receive heartbeat from PC via the USB
  received = "";
  received = Serial.readStringUntil(terminator);

  processReceived();
  
  // Check if we have not been getting a heartbeat for longer than the TRIGGER period and trigger reset via OC
  sprintf(buf, "DEBUG: TimeoutCnt: %d", cnt);
  Serial.println(buf);

  // Only trigger reboot if at least one message has been received (armed==1)
  //   or unarmed and more than .5 hour has passed ~ 
  //   or forced - button pressed
  if ((cnt >= HBTRIGGERDELAY && armed == 1) || (cnt >= HB_UNARMED_TRIGGERDELAY && armed == 0) || (force == 1)) {
    Serial.println("DEBUG: FIRING RESET!");
    multiBlink(2, 150, 50);          // Two short blinks of GREEN LED before the RESET
    digitalWrite(OC, HIGH);
    delay(2000);                     // wait for a second or two
    Serial.println("DEBUG: DONE.!");
    digitalWrite(OC, LOW);
    //    delay(30000);              // wait for 30 seconds - the PC is doing the reset... so it might take some time before we continue anyway
    cnt = 0;
    cntHB = 0;
    force = 0;
    armed = 0;
    Serial.println("DEBUG: Reseting the board.");
    softwareReset();
  }

#ifdef BOARD_MICROVIEW
  uView.circleFill(32, 24, 10, BLACK, NORM);
  uView.display();
#endif
  delay(1000);               // wait for a second
} /* void loop() */
