# Fuji Battleships
This is a WIP cross platform game client for the Fuji Battleships server.



### Supported Platforms
* **CoCo**
* **Atari** *(planned)*
* **Apple II** *(planned)*
* **C64** *(planned)*
* *Please contribute to add more!*

### To Build and Test in emulator
1. Start Fujinet-PC for your appropriate platform
2. Review/edit the Post Build and  the emulator start commands in `Makefile`
3. To build and test: `make [platform] mount test`
Example:`make coco test`



### To build:

`make [platform]`

The "Ready 2 Run" output files will be in `./r2r`, which can be copied to a TNFS server, etc.

This project uses the MekkoGX Makefiles platform, which should automatically download any dependencies.


# Server / Api details

Please visit the server page for more information:

https://github.com/FujiNetWIFI/servers/tree/main/fujinet-game-system/battleship#readme
