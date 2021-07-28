#include <Arduino.h>

void print(String string, bool firstRow, int paddingLeft);
void displayMode();
void renderDisplay();
bool handleButton();
bool delay_with_button(size_t time);