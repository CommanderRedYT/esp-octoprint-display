# esp-octoprint-lcd
This project is for displaying informations about your 3d-printer via the Octoprint API.

It uses a **ESP8266** *(tested)* or **ESP32** *(not tested)* for controlling a 16x2 LCD via I²C. It also has a push button to switch to the next value or start the looping mode.

# Modes
- Normal
  - Press the button to switch to the next value.
  - Current implemented values:
    - Current State (Printing, Operational,...)
    - Current Filename
    - Print time left
    - Print Percentage
    - Nozzle temp.
    - Bed temp.
  - Display will go dark after a few seconds.
- Loop
  - Access this mode by pressing and holding the button.
  - Displays all values accessible by the Normal-Mode one after another. 
  - Press the button again to exit this mode.

# Configure
To configure the code, go into `include/credentials.h`. There you can find all of the available options.

# Images
![Image 1](/images/1.jpg?raw=true)
![Image 2](/images/2.jpg?raw=true)
![Image 3](/images/3.jpg?raw=true)
![Image 4](/images/4.jpg?raw=true)