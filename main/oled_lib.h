#include <stdbool.h>
#include <stdint.h>
#include "font_struct.h"

#define OLED_ADDR 0x3C
#define MASTER_CLK_SPEED 400000
#define RX_BUF_DISABLE 0
#define TX_BUF_DISABLE 0
#define ESP_INTR_FLAG_DEFAULT 0
#define ACK_CHECK_EN 0x01
#define ACK_CHECK_DIS 0x00

void oled_cmd(uint8_t cmd_code);
void oled_data(uint8_t * data, uint32_t size);
void oled_init();
void oled_set_pixel(int x, int y);
void oled_draw_line(int x1, int y1, int x2, int y2);
void oled_draw_rectangle(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h);
void oled_draw_filled_rectangle(int x0, int y0, int w, int h);
void oled_draw_circle(int x_c, int y_c, int r);
void oled_draw_filled_circle(int x_c, int y_c, int r);
void oled_set_font(const FontDesc * fDescs, const FontInfo * fInfo, const uint8_t * fBitmap);
void oled_draw_char(char c, int x0, int y0);
void oled_draw_text(char * c, int x0, int y0, bool wrap);
void oled_draw_bitmap(uint8_t * bitmap, int x0, int y0);
void oled_draw_ellipse(int x_c, int y_c, int a, int b);
void oled_display();
void oled_display2();
void oled_clear();