###############################################################################
# 
# PC Crash Watchdog
#    Utilising Arduino and Optocoupler
#
# Author: Milan Štubljar of Štubljar d.o.o. (milan@stubljar.com)
#		  Phone: +386 31 318 848
#
# About:
#	The solution consist of 2 components:
#		- Hardware board with Arduino (Nano) with preloaded PC Crash Watchdog.
#		- A PC Crash Watchdog Windows Powershell Script (pc-crash-watchdog.ps1)
#
# Instructions:
#	1. Setup the board connecting it to one of your USB ports.
# 	   Depending on the board (Nano) it might require special drivers.
#	   For example: CH341SER.zip which you can download from:
#	   See details and instructions here: 
# 		  https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers/windows-710
#	2. Determine the COM port and update the pc-crash-watchdog.ps1
#		Edit the line: $comPort    = 'COM5'    # Set to COM port of the PC WATCHDOG Device
#		setting the right port number.
#	3. Having the board connected to USB and to your PC reset PIN, run the pc-crash-watchdog.ps1
#
#   Notes:
#	Once the board is powered it will start a countdown of 30 mins (about) and will trigger 
#	reset if during this time no "Alive" message via COM port is received. The alive message
#	arms the board and resets the reset countdown to 60 sec.
#   Two ways to prevent this to happen:
#	1. Reset the board to reset the counter.
# 	2. Run the pc-crash-watchdog.ps1 which continusuly sends the alive message and keeps the counter to 0.
#
#   Once the pc-crash-watchdog.ps1 sends its first message (alive) to the board, the board
#	gets armed. In armed mode, the reset of PC will be initiated in 60 secounds after the last
#   alive message. The pc-crash-watchdog.ps1 sends the alive message every second, board indicates 
#	the alive message was received by GREEN led single blink.
#   In the event of PC freeze, the reset will be initiated within a minute or so.
#   Prior the reset, the following led sequnce will rollout:
#		- Two short blinks of GREEN led.
#		- One long RED blink (this is hardwired to the reset so reset is being initiated while red LED is lit.
#   The board blinks 3 times on startup (after a powerup or reset).
#	The reset (programatic or hard via button press on the board) will put the board into disarmed mode.
#


# How to run the PC Watchdog Windows Powershell script from within a batch:
:: Start PC Crash Watchdog
echo Starting PC Crash Watchdog
start "PC Crash Watchdog" /D "C:\dev\Tools\Monitor" pc-crash-watchdog.cmd

