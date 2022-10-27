### Supported engine versions: 
- UE 4.23-4.27
Edit the 'engine.cpp' file to add support for your game.
### Currently supported games:
 - Back 4 Blood
 - Boundary
 - Brickadia
 - Dauntless
 - Dead By Daylight
 - Fortnite
 - POLYGON
 - Rogue Company
 - SCUM
 - Scavengers
 - Splitgate
 - The Cycle: Frontier
 - The Isle
 - Witch It
### Usage:
```
.\Dumper.exe [option] [option]
```
```
Options:
  '-h' - prints help message
  '-p' - dump only names and objects
  '-w' - wait for input
  '-f packageNameHere' - specifies package where we should look for pointers in paddings (can take a lot of time)
```
### Todo:
- Analyze functions to get offsets to referenced fields
