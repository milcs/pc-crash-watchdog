############################################
# PC Crash Watchdog
#  v2.04 Milc 2024-01-25 Added COM Port print
#  v2.03 Milc 2021-11-28 Added retrySleep param
#  v2.02 Milc 2021-11-27 Added command line param -reboot_now
#  v2.01 Milc 2021-11-26 Sports new output logging with date and msg. severity
#                        write serial enclosed in try/catch and reopen
#                        CMD Window title added
#                        Code cleanup
#                        Messages in colors...
#  v2.00 Milc 2021-11-24 Minor port change
#
#  v1.00 Milc 2018-09-08 Initial version
#
#  Author: Milan Stubljar of Stubljar d.o.o. (milan@stubljar.com)

# Command Line Params Handling
Param(
    [Parameter(Mandatory=$false)][ValidateSet("true", "false", "yes", "no", "0", "1")][string]$reset_now="false"
)

$forceReset = $false
switch($reset_now.ToLower()) {
    "1" { $forceReset = $true }
    "yes" { $forceReset = $true }
    "true" { $forceReset = $true }
    default { $forceReset = $false }
}

#
# GLOBAL SETTINGS:
#

$version    = "PC Crash Watchdog v2.04"   # Version String
$environment= 'Development'
$comPort    = 'COM3'    # Set to COM port of the PC WATCHDOG Device
$maxRetry   = 50        # Max Retries
$retrySleep = 5000      # Delay between Retries
$pingSleep  = 1000      # Delay between PINGs in ms

#
# INTERALS - DO NOT CHANGE
#

$hbAliveMsg = 'I am alive.' # . is a terminator!!!
$hbRebootMsg = 'Simply Reboot.'
$openCnt = 0
$isOpen = $false
$port= new-Object System.IO.Ports.SerialPort $comPort,38400,None,8,one

function Write-Log {
    [CmdletBinding()]
    param(
        [Parameter()]
        [ValidateNotNullOrEmpty()]
        [string]$Message,

        [Parameter()]
        [ValidateNotNullOrEmpty()]
        [ValidateSet('Information','Warning','Error','Critical','Debug')]
        [string]$Severity = 'Information',

        [Parameter()]
        [System.ConsoleColor]$ForegroundColor = 'Gray'
    )

    [hashtable]$levels = [ordered]@{
        Information = 'INFO';
        Error = "ERROR";
        Debug = 'DEBUG';
        Warning = 'WARN';
        Critical = 'CRITICAL'
    }

    if ($False)
    {
        [pscustomobject]@{
            Time = (Get-Date -f "yyyy-MM-dd HH:mm:ss.ffff")
            Message = $Message
            Severity = $Severity
        } | Export-Csv -Path "$env:Temp\LogFile.csv" -Append -NoTypeInformation
    }
    $Time = (Get-Date -f "yyyy-MM-dd HH:mm:ss.ffff")
    Write-Host "$Time [$($levels[$Severity])] $Message" -ForegroundColor $ForegroundColor
 } # Write-Log

function Open-Serial()
{
    try 
    {
        $port.Open()
        return $true
    }
    catch
    {
      Write-Log -Message $_.Exception.Message -Severity "Error" -ForegroundColor Red
    }
    return $false
} # Open-Serial

# Open Serial Port with retry
function Open-Serial-Port()
{
    while (-Not($isOpen = Open-Serial) -and ($openCnt -lt $maxRetry))
    {
        $openCnt += 1
        Write-Log "Retrying in 5 sec." -Severity "Error" -ForegroundColor Red
        Start-Sleep -Milliseconds $retrySleep
    }

    if ($isOpen -eq $false)
    {
        Write-Log "Could not open serial port. Aborting." -Severity "Error" -ForegroundColor Red
        exit
    }
    Write-Log "Serial port $comPort succesfully opened." -ForegroundColor Green
} # Open-Serial-Port

function Read-Serial()
{
    # Attempt to read from serial
    $stopwatch = New-Object System.Diagnostics.Stopwatch
    $stopwatch.Start()
    while ($stopwatch.ElapsedMilliseconds -lt 1000)
    {
        if ($port.BytesToRead -gt 0)
        {
            $so = $port.ReadLine()
            Write-Log "Received: $so"
        }
        Start-Sleep -Milliseconds 100
    }
    $stopwatch.Stop()
} # Read-Serial


# MAIN
# MAIN
# MAIN

try {
	if ($environment -eq "Development")
	{
		$host.UI.RawUI.WindowTitle = "$version $environment @ $comPort"
	} else {
		$host.UI.RawUI.WindowTitle = $version
	}
    Write-Log "$version by Milc 2018-09-08" -ForegroundColor Cyan

    Write-Log "ForceReset (-reset_now): $forceReset" -ForegroundColor Magenta

    Open-Serial-Port
    [console]::TreatControlCAsInput = $true

    if ($forceReset)
    {
        Write-Log "Sending 'Alive' message." -ForegroundColor Green
        $port.Write($hbAliveMsg)

        Read-Serial
        Start-Sleep -Milliseconds 1000

        Write-Log "Sending 'Reboot' message." -ForegroundColor Green
        $port.Write($hbRebootMsg)
        $port.Close()

        Write-Log "Done."
        Write-Log "Exiting in 10s."
        Start-Sleep -Milliseconds 10000
        exit
    }

    $n = 0
    while ($true) {
        $n+=1

        # Output the version string
		if ($n % 10 -eq 0) {
			Write-Host " "
			Write-Log $version -ForegroundColor Cyan
		}

        # Send - WRITE to serial port, sending the ALIVE message
        Write-Log "Loop $($n): Sending 'Alive' message on $comPort port." -ForegroundColor Green

#Exception calling "Write" with "1" argument(s): "The port is closed."
#At C:\src\PycharmProjects\Arduino\Milan-Stubljar\Monitor\pc-crash-watchdog.ps1:75 char:9
#+         $port.Write($hbAliveMsg)
#+         ~~~~~~~~~~~~~~~~~~~~~~~~
#    + CategoryInfo          : NotSpecified: (:) [], MethodInvocationException
#    + FullyQualifiedErrorId : InvalidOperationException

        try
        {
            $port.Write($hbAliveMsg)
            # Write-Host $so
        } catch {
            Write-Log -Message $_.Exception.Message -Severity "Error"
            Write-Log "Attempting to re-open serial port."
            Open-Serial-Port
            Write-Log "Seems we are back in the game. Continuing..."
            continue
        }

        Read-Serial

        # Check if CTRL-C was pressed and exit gracefully
        if ([Console]::KeyAvailable) {
          $key = [Console]::ReadKey($true)
          if ($key.key -eq "C" -and $key.modifiers -eq "Control") { 
            Write-Log "Detected Ctrl-C. Housekeeping and exiting."
            $port.Close()
            exit
          }
        }

        # Wait some time before next PING
        Write-Log "Sleeping for $($pingSleep/1000)s"
        Start-Sleep -Milliseconds $pingSleep

    } # while ($true)
    $port.Close()

} finally {

   # No matter what the user did, reset the console to process Ctrl-C inputs 'normally'
    [console]::TreatControlCAsInput = $false

    if ($isOpen -eq $false) {
        $port.Close()
    }
}

# Last line, well almost.
