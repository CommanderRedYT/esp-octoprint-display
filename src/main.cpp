#include "credentials.h"

#include <OctoPrintAPI.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#include <TimeLib.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif
#ifdef ESP32
#include <WiFi.h>
#endif
#include <WiFiClient.h>

#include "helpers.h"

WiFiClient client;

#ifndef USEHOSTNAME
OctoprintApi api(client, ip, octoprint_httpPort, octoprint_apikey);
#else
OctoprintApi api(client, octoprint_host, octoprint_httpPort, octoprint_apikey);
#endif

unsigned long api_mtbs = 5000;
unsigned long api_lasttime = 0;

int current_mode = 0;
int last_mode = 0;
int MAX_MODES = 5;

int displayTimer = 0;

bool lastState = false;
bool state = false;

bool displayOn = false;

bool renderFlag = true;

bool constant_mode = false;
unsigned int lastConstantTime = 0;
unsigned int lastConstantUpdate = 0;

int mainTimeout = 0;

void setup()
{
  lcd.init();
  lcd.setCursor(0, 0);
  lcd.backlight();
  lcd.print("Booting...");
  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  pinMode(BUTTON, INPUT_PULLUP);
  lcd.clear();
  lcd.noBacklight();
}

void print(String string, bool firstRow = true, int paddingLeft = 0)
{
  if (string.length() + paddingLeft <= 16 || constant_mode == true)
  {
    lcd.setCursor(paddingLeft, !firstRow);
    if (constant_mode)
      lcd.print(string.substring(0, 16));
    else
      lcd.print(string);
  }
  else
  {
    for (size_t i = 0; i < string.length() - 15; i++)
    {
      String now = string.substring(i, i + 16);
      lcd.setCursor(0, !firstRow);
      lcd.print(now);
      if (i == 0)
        if (delay_with_button(400))
        {
          return;
        }
      if (delay_with_button(220))
      {
        return;
      }
    }
    if (delay_with_button(820))
    {
      return;
    }
  }
}

void displayMode()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Mode: (");
  lcd.print(current_mode);
  lcd.print(")");
  lcd.setCursor(0, 1);
  switch (current_mode)
  {
  case 0:
    print("Current State", false);
    break;

  case 1:
    print("Current Filename", false);
    break;

  case 2:
    print("Print time left", false);
    break;

  case 3:
    print("Print Percentage", false);
    break;

  case 4:
    print("Nozzle temp.", false);
    break;

  case 5:
    print("Bed temp.", false);
    break;

  default:
    print("Undefined", false);
    break;
  }
}

void renderDisplay()
{
  renderFlag = false;
  lcd.clear();
  switch (current_mode)
  {
  case 0:
  {
    print("Current State:");
    print(api.printJob.printerState, false);
    break;
  }
  case 1:
  {
    print("Current File:");
    print(api.printJob.jobFileName, false);
    break;
  }
  case 2:
  {
    print("Print time left:");
    int runHours = api.printJob.progressPrintTimeLeft / 3600;
    int secsRemaining = api.printJob.progressPrintTimeLeft % 3600;
    int runMinutes = secsRemaining / 60;
    int runSeconds = secsRemaining % 60;
    char time_left_buf[10];
    sprintf(time_left_buf, "%02d:%02d:%02d", runHours, runMinutes, runSeconds);
    print(time_left_buf, false);
    break;
  }
  case 3:
  {
    print("Percentage:");
    const float temp_percent = floor(api.printJob.progressCompletion * 100) / 100;
    char percent_buf[8];
    sprintf(percent_buf, "%.2f%%", temp_percent);
    print(percent_buf, false);
    break;
  }
  case 4:
  {
    print("Nozzle temp.");
    const float nozzleActual = api.printerStats.printerTool0TempActual;
    const float nozzleTarget = api.printerStats.printerTool0TempTarget;
    char nozzleBuf[22];
    sprintf(nozzleBuf, "%.2fC/%.2fC", nozzleActual, nozzleTarget);
    print(nozzleBuf, false);
    break;
  }
  case 5:
  {
    print("Bed temp.");
    const float bedActual = api.printerStats.printerBedTempActual;
    const float bedTarget = api.printerStats.printerBedTempTarget;
    char bedBuf[22];
    sprintf(bedBuf, "%.2fC/%.2fC", bedActual, bedTarget);
    print(bedBuf, false);
    break;
  }
  default:
    break;
  }
}

bool handleButton()
{
  state = !digitalRead(BUTTON);
  if (state != lastState)
  {
    lastState = state;
    if (state)
    {
      if (constant_mode)
      {
        constant_mode = !constant_mode;
        return true;
      }
      int timer = 200;
      while (timer > 1)
      {
        timer--;
        delay(1);
        if (digitalRead(BUTTON))
          break;
      }
      if (digitalRead(BUTTON))
      {
        displayTimer = 16500;
        if (displayOn == false)
        {
          displayOn = true;
          lcd.backlight();
          renderDisplay();
        }
        else
        {
          current_mode++;
          if (current_mode > MAX_MODES)
            current_mode = 0;
        }
      }
      else
      {
        constant_mode = !constant_mode;
      }
      return true;
    }
  }
  return false;
}

bool delay_with_button(size_t time)
{
  for (size_t delayWithButton = 0; delayWithButton < time; delayWithButton++)
  {
    delay(1);
    if (!digitalRead(BUTTON))
    {
      mainTimeout = 0;
      return true;
    }
  }
  return false;
}

void loop()
{
  if (constant_mode)
  {
    handleButton();
    if (millis() - lastConstantTime > 5000)
    {
      lastConstantTime = millis();
      current_mode++;
      if (current_mode > MAX_MODES)
        current_mode = 0;
    }

    if (millis() - api_lasttime > 1000 || api_lasttime == 0)
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        if (api.getPrintJob() && api.getPrinterStatistics())
        {
          renderDisplay();
        }
      }
      api_lasttime = millis();
    }
  }
  else
  {

    if (displayTimer > 0)
    {
      displayTimer--;
      delay(1);
      if (displayTimer == 1)
      {
        lcd.clear();
        lcd.noBacklight();
        displayOn = false;
      }
    }
    handleButton();

    if (current_mode != last_mode)
    {
      renderFlag = true;
      last_mode = current_mode;
      mainTimeout = 500;
      displayMode();
      api_lasttime = 0;
    }
    if (mainTimeout == 1)
      lcd.clear();
    if (mainTimeout > 0)
    {
      delay(1);
      mainTimeout--;
    }
    else
    {
      if (millis() - api_lasttime > api_mtbs || api_lasttime == 0)
      {
        if (WiFi.status() == WL_CONNECTED)
        {
          if (api.getPrintJob() && api.getPrinterStatistics())
          {
            if (displayOn == true)
            {
              renderDisplay();
            }
          }
        }
        api_lasttime = millis();
      }
    }
  }
}