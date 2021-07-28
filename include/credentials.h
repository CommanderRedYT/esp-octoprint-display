#include <IPAddress.h>
const char *ssid = "my-ssid";        // your network SSID (name)
const char *password = "my-wifi-password"; // your network password

#define BUTTON D5 //Pin where the push button is connected

//#define USEHOSTNAME //Define if you want to use Hostname instead of IP for octoprint api

// You only need to set one of the of follwowing:
#ifndef USEHOSTNAME
IPAddress ip(192, 168, 0, 1); // Your IP address of your OctoPrint server (internal or external)
#else
char* octoprint_host = "octoprint.example.com";  // Or your hostname. Comment out one or the other.
#endif

const int octoprint_httpPort = 80; //Port where octoprint webserver lives
String octoprint_apikey = "api-key"; //The octoprint api key
