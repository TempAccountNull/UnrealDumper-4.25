### Supported engine versions: 
- UE 4.23-4.27
- Edit the 'engine.cpp' file to add support for your game.
### Currently supported games:
 - Back 4 Blood
 - Boundary
 - Brickadia
 - Dauntless
 - Dead By Daylight
 - Fortnite
 - Foxhole
 - Hogwarts Legacy
 - Icarus
 - Lunch Lady
 - Mordhou
 - POLYGON
 - Ready or Not
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

### Shouts
- https://github.com/DDbGA/UnrealDumper-4.25 (for Ready or Not, Foxhole, and Icarus)
- https://github.com/Suwup/UnrealDumper-4.25 (for Mordhou)
- https://github.com/Sukooo/UnrealDumper-4.25 (for Lunch Lady)
