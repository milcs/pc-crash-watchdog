# PC Crash Watchdog
Utilising Arduino and Optocoupler

Author: [Milan Štubljar of Štubljar d.o.o.](milan@stubljar.com)

## About
PC Crash Watchdog consist of two components:
- Hardware board with Arduino (Nano) with preloaded PC Crash Watchdog.
- A PC Crash Watchdog Windows Powershell Script (pc-crash-watchdog.ps1)

![PC Crash Watchdog - Board.png](img%2FPC%20Crash%20Watchdog%20-%20Board.png)

## Instructions
### Setup
1. Setup the board connecting it to one of your USB ports. Depending on the board (Nano) it might require special drivers. For example: CH341SER.zip - Download [details and instructions](https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers/windows-710). The ino sketch can be found in pc-crash-watchdog-arduino folder.

3. Determine the COM port and update the ```pc-crash-watchdog.ps1```:

Edit the line: ```$comPort    = 'COM5'    # Set to COM port of the PC WATCHDOG Device```
setting the right port number.

4. Having the board connected to USB and to your PC reset PIN, run the ```pc-crash-watchdog.ps1```

###  Hardware Manifest
- 1 x Breadboard
- 1 x Arduino [Nano](https://store.arduino.cc/products/arduino-nano) 
- 1 x Optocoupler - [4N35](https://www.componentsinfo.com/4n35-optocoupler-pinout-datasheet/)
- 2 x Resistor 220 Ω (5 band: red, red, black, black, brown), 5th color - Tolerance can also be 2% (red) or 5% (gold)
- 1 x Push button
- 2 x LED (green, red)

Wire the board as per Board image.

## Notes:
Once the board is powered it will start a countdown of 30 mins (about) and will trigger 
reset if during this time no "Alive" message via COM port is received. The alive message
arms the board and resets the reset countdown to 60 sec.

Two ways to prevent this to happen:
1. Reset the board to reset the counter.
2. Run the pc-crash-watchdog.ps1 which continusuly sends the alive message and keeps the counter to 0.

Once the ```pc-crash-watchdog.ps1``` sends its first message (alive) to the board, the board
gets armed. In **armed mode**, the reset of PC will be initiated in 60 secounds after the last
alive message. The ```pc-crash-watchdog.ps1``` sends the alive message every second, board indicates 
the alive message was received by GREEN led single blink.
In the event of PC freeze, the reset will be initiated within a minute or so.

Prior the reset, the following led sequnce will rollout:
- Two short blinks of GREEN led.
- One long RED blink (this is hardwired to the reset so reset is being initiated while red LED is lit.

The board blinks 3 times on startup (after a powerup or reset). 

The reset (programatic or hard via button press on the board) will put the board into **disarmed mode**.

## How to run PC Watchdog Windows Powershell script from within a batch file:
```cmd
@echo off
setlocal
::Running watchdog
del /S pc-crash-watchdog.log >nul 2>&1
:: powershell.exe -File pc-crash-watchdog.ps1
pwsh -File pc-crash-watchdog.ps1

endlocal
exit
```
![PC Crash Watchdog v2.04 Development Screen.png](img%2FPC%20Crash%20Watchdog%20v2.04%20Development%20Screen.png)

## Circuit
![PC-Crash- Watchdog-Circuit.jpg](img%2FPC-Crash-%20Watchdog-Circuit.jpg)
