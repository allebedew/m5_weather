
#include "display.h"

#define BLK_T_LEN   10

const int w = 4, h = 6;
const int block_sp = 5;
const int block_w = 75;
const int block_h = 35;

struct block {
  char title[BLK_T_LEN], value[BLK_T_LEN];
  uint16_t text_color, back_color;
  unsigned long updated;
  bool silent;
};

struct block *blocks = (struct block*) malloc(w * h * sizeof(struct block));

void draw_blocks_if_needed();
void dump_blocks();

void display_setup() {
  for (int i=0; i<w * h; i++) {
    blocks[i].title[0] = '\0';
    blocks[i].value[0] = '\0';
    blocks[i].text_color = TFT_WHITE;
    blocks[i].back_color = TFT_BLACK;
    blocks[i].updated = 0;
    blocks[i].silent = false;
  }
}

void display_loop() {
    draw_blocks_if_needed();
}

void dump_blocks() {
  for (int y=0; y<h; y++) {
    Serial.printf("\n%d: ", y);
    for (int x=0; x<w; x++) {
      struct block* b = &blocks[x + y * w];
      Serial.printf("[\"%s\" \"%s\" %x] ", b->title, b->value, b->text_color);
    }
  }
  Serial.printf("\n\n");
}

void update_block(int number, const char* title, const char* value, uint32_t text_color, uint32_t back_color, bool silent) {
  struct block* b = blocks + number;
  if (strcmp(title, b->title) == 0 && strcmp(value, b->value) == 0 && 
      text_color == b->text_color && back_color == b->back_color) return;
  strlcpy(b->title, title, BLK_T_LEN);
  strlcpy(b->value, value, BLK_T_LEN);
  b->text_color = text_color;
  b->back_color = back_color;
  b->updated = millis();
  b->silent = silent;
}

void draw_blocks_if_needed() {
  for (int y=0; y<h; y++) {  // 320x240
    for (int x=0; x<w; x++) {
      struct block* b = blocks + (x + y * w);

      if (b->updated == 0) continue;
      
      int blk_x = x * (block_w + block_sp);
      int blk_y = y * (block_h + block_sp);

      if (millis() - b->updated < 500 && !b->silent) {
        M5.Lcd.fillRect(blk_x, blk_y, block_w, block_h, TFT_DARKGREY);
      } else {
        M5.Lcd.fillRect(blk_x, blk_y, block_w, block_h, b->back_color);
        b->updated = 0;
      }

      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(TFT_LIGHTGREY);
      M5.Lcd.setTextDatum(TC_DATUM);
      M5.Lcd.drawString(b->title, blk_x + block_w / 2, blk_y + 5);

      M5.Lcd.setTextSize(2);
      M5.Lcd.setTextColor(b->text_color);
      M5.Lcd.setTextDatum(BC_DATUM);
      M5.Lcd.drawString(b->value, blk_x + block_w / 2, blk_y + block_h - 1);
    }
  }
}
