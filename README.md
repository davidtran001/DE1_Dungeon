<img src="https://github.com/davidtran001/DE1_Dungeon/blob/main/images/title.png?raw=true">

# Description
DE1 Dungeon is a 2D top-down action/survival game. The player controls a fearless knight
that uses his golden dagger to defend himself. Ghost will appear on the map and chase the
player around. In order to defend against the ghost, daggers can be thrown at enemy ghosts
and explosive barrels. When a dagger hits a barrel it will explode, damaging all entities within the explosion's radius. 
Once the player’s health bar is completely red then it is game over. The goal of
the game is to kill ghosts and survive as long as you can.

# Gameplay
<img src="https://github.com/davidtran001/DE1_Dungeon/blob/main/images/gameplay.gif?raw=true">
<img src="https://github.com/davidtran001/DE1_Dungeon/blob/main/images/gameplay.png?raw=true">
<img src="https://github.com/davidtran001/DE1_Dungeon/blob/main/images/gameplay2.png?raw=true">

# How to run DE1 Dungeon
**There are two methods of running DE1 Dungeon**
1. Intel FPGA Monitor Program
- Turn on DE1-SoC 
- Create new project for DE1-SoC to run C code
- Add the c files and download system
- Compile and Run
- Press play button and switch to the monitor input to the DE1-SoC VGA port
- Connect PS/2 keyboard

2. [CPUlator](https://cpulator.01xz.net/?sys=arm-de1soc)
- Open the main.c file
- Press Compile
- Press Continue
- Press the PS/2 keyboard box
- View the game through the VGA Pixel buffer, the direction you moved through
LEDs and the points you got through Seven-segment displays.

# How to play
- 'W' to move up
- 'A' to move left
- 'S' to move down
- 'D' to move right
- 'I' to throw dagger
