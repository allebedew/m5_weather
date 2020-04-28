
#include <M5Stack.h>

void display_setup();
void display_loop();

void update_block(int number, const char* title, const char* value, uint32_t text_color = TFT_WHITE, uint32_t back_color = TFT_BLACK, bool silent = false);