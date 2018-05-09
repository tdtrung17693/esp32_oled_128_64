#include "oled_lib.h"
#include "driver/i2c.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "string.h"
// #include "envyCodeR16pt.h"
// #include "sourceHanSansJP16pt.h"
// #include "freeSans13pt.h"
// #include "tamzen5_9b.h"
#include "esp_log.h"
#include <inttypes.h>

#include "unicode_helper.h"

// const FontDesc * fontDescs = fontSourceHanSansJPNormal16ptDescriptors;
// const FontInfo * fontInfo = &fontSourceHanSansJPNormal16ptInfo;
// const uint8_t * fontBitmap = fontSourceHanSansJPNormal16pt;
// const FontDesc * fontDescs = fontEnvyCodeR16ptDescriptors;
// const FontInfo * fontInfo = &fontEnvyCodeR16ptInfo;
// const uint8_t * fontBitmap = fontEnvyCodeR16pt;
// const FontDesc * fontDescs = fontFreeSans13ptDescriptors;
// const FontInfo * fontInfo = &fontFreeSans13ptInfo;
// const uint8_t * fontBitmap = fontFreeSans13pt;
static FontDesc * fontDescs;
static FontInfo * fontInfo ;
static uint8_t * fontBitmap ;
// const FontDesc * fontDescs = fontunifont15ptDescriptors;
// const FontInfo * fontInfo = &fontunifont15ptInfo;
// const uint8_t * fontBitmap = fontunifont15pt;


void oled_set_font(const FontDesc * fDescs, const FontInfo *fInfo, const uint8_t * fBitmap) {
    fontDescs = (FontDesc *)fDescs;
    fontInfo = (FontInfo *)fInfo;
    fontBitmap = (uint8_t *)fBitmap;
}

void get_text_dim(char * text, uint32_t * ret) {
    uint32_t w = 0;
    uint32_t max_asc = 0;
    uint32_t max_desc = 0;
    uint32_t text_len = strlen((const char *)text);
    uint32_t charDescIndex ;
    int32_t codePoint;
    uint32_t ret_cp[2];
    FontDesc glyph;

    for (int i = 0; i < text_len; ++i) {
        codePoint = to_cp(&text[i], ret_cp);
        charDescIndex = codePoint - fontInfo->first_char;
        glyph = fontDescs[charDescIndex];

        max_asc = fmax(max_asc, glyph.ascent);
        max_desc = fmax(max_desc, glyph.descent);
        w += glyph.advance_width;

        i += (ret_cp[0] - 1);
    }

    uint32_t height = max_asc + max_desc;

    ret[0] = w;
    ret[1] = height;
    ret[2] = max_desc;
}


void i2c_init() {
    i2c_config_t cfg;

    cfg.mode = I2C_MODE_MASTER;
    cfg.sda_io_num = 21;
    cfg.scl_io_num = 22;
    cfg.sda_pullup_en = GPIO_PULLUP_ENABLE;
    cfg.scl_pullup_en = GPIO_PULLUP_ENABLE;
    cfg.master.clk_speed = MASTER_CLK_SPEED;

    i2c_param_config(I2C_NUM_1, &cfg);
    i2c_driver_install(
        I2C_NUM_1,
        cfg.mode,
        RX_BUF_DISABLE,
        TX_BUF_DISABLE,
        ESP_INTR_FLAG_DEFAULT
    );
}

void oled_cmd(uint8_t cmd_code) {
    uint8_t p[2] = { 0x00, cmd_code };
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, OLED_ADDR << 1, ACK_CHECK_EN);

    i2c_master_write(cmd, p, 2, ACK_CHECK_EN);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_ERR_TIMEOUT) {
        printf("I2C timeout\n");
    }

    if (ret == ESP_FAIL) {
        printf("I2C Fail\n");
    }
}

void oled_data(uint8_t * data, uint32_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, OLED_ADDR << 1 , ACK_CHECK_EN);

    i2c_master_write_byte(cmd, 0x40, ACK_CHECK_EN);
    i2c_master_write(cmd, data, len, ACK_CHECK_EN);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_ERR_TIMEOUT) {
        printf("I2C timeout\n");
    }

    if (ret == ESP_FAIL) {
        printf("I2C Fail\n");
    }
}

DMA_ATTR static uint8_t SSD1306_Buffer[128 * 64 / 8];

void oled_set_pixel(int x, int y) {
    if (x < 0 || y < 0 || x > 127 || y > 63) return;
    SSD1306_Buffer[ x + (y / 8) * 128 ] |= (1 << (y % 8));
}

void oled_draw_line(int x1, int y1, int x2, int y2) {
    int x, y, error, delta, schritt, dx, dy, inc_x, inc_y;

    x = x1;
    y = y1;

    dy = y2 - y1;
    dx = x2 - x1;

    if (dx > 0) {
        inc_x = 1;
    } else {
        inc_x = -1;
    }

    if (dy > 0) {
        inc_y = 1;
    } else {
        inc_y = -1;
    }

    if (abs(dy) < abs(dx)) {
        error = -abs(dx);
        delta = 2 * abs(dy);
        schritt = 2 * error;
        while (x != x2) {
            oled_set_pixel(x, y);
            x += inc_x;
            error = error + delta;
            if (error > 0) {
                y += inc_y;
                error += schritt;
            }
        }
    } else {
        error = -abs(dy);
        delta = 2 * abs(dx);
        schritt = 2 * error;

        while (y != y2) {
            oled_set_pixel(x, y);
            y += inc_y;
            error = error + delta;
            if (error > 0) {
                x += inc_x;
                error += schritt;
            }
        }
    }

    oled_set_pixel(x2, y2);
}

void oled_draw_rectangle(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h) {
    for (int x = x0; x <= x0 + w; ++x) {
        oled_set_pixel(x, y0);
        oled_set_pixel(x, y0 + h);
    }

    for (int y = y0; y <= y0 + h; ++y) {
        oled_set_pixel(x0, y);
        oled_set_pixel(x0 + w, y);
    }
}

void oled_draw_filled_rectangle(int x0, int y0, int w, int h) {
    for (int x = x0; x < x0 + w; ++x) {
        for (int y = y0; y < y0 + h; ++y) {
            oled_set_pixel(x, y);
        }
    }
}

// Bresenham-Algorithmus für den vollen Kreis
void oled_draw_circle(int x_c, int y_c, int r) {
    int x = x_c, y = y_c;
    int xh, yh, d, dx, dxy;

    xh = 0;
    yh = r;
    d = 1-r;
    dx = 3;
    dxy = -2*r + 5;

    while(yh >= xh) {
        oled_set_pixel(x+xh, y+yh);
        oled_set_pixel(x+yh, y+xh);
        oled_set_pixel(x+yh, y-xh);
        oled_set_pixel(x+xh, y-yh);
        oled_set_pixel(x-xh, y-yh);
        oled_set_pixel(x-yh, y-xh);
        oled_set_pixel(x-yh, y+xh);
        oled_set_pixel(x-xh, y+yh);


        if (d < 0) {
            d+=dx;
            dx+=2;
            dxy+=2;
            xh++;
        } else {
            d+=dxy;
            dx+=2;
            dxy+=4;
            xh++;
            yh--;
        }
    }
}

void oled_draw_filled_circle(int x_c, int y_c, int r) {
    int x = x_c, y = y_c;
    int xh, yh, d, dx, dxy;

    xh = 0;
    yh = r;
    d = 1-r;
    dx = 3;
    dxy = -2 * r + 5;

    while (yh >= xh) {
      oled_draw_line(x - xh, y + yh, x + xh, y + yh);
      oled_draw_line(x - yh, y + xh, x + yh, y + xh);
      oled_draw_line(x - yh, y - xh, x + yh, y - xh);
      oled_draw_line(x - xh, y - yh, x + xh, y - yh);

      if (d < 0) {
        d += dx;
        dx += 2;
        dxy += 2;
        xh++;
      } else {
        d += dxy;
        dx += 2;
        dxy += 4;
        xh++;
        yh--;
      }
    }
}

// Bresenham-Algorithmus für die Ellipse
void oled_draw_ellipse(int x_c, int y_c, int a, int b) {
    int dx = 0, dy = b; /* im I. Quadranten von links oben nach rechts unten */
    long a2 = a*a, b2 = b*b;
    long err = b2-(2*b-1)*a2, e2; /* Fehler im 1. Schritt */

    do {
        oled_set_pixel(x_c + dx, y_c + dy); /* I. Quadrant */
        oled_set_pixel(x_c - dx, y_c + dy); /* II. Quadrant */
        oled_set_pixel(x_c - dx, y_c - dy); /* III. Quadrant */
        oled_set_pixel(x_c + dx, y_c - dy); /* IV. Quadrant */

        e2 = 2*err;
        if (e2 <  (2*dx+1)*b2) {
            dx++;
            err += (2*dx+1)*b2;
        }
        if (e2 > -(2*dy-1)*a2) {
            dy--;
            err -= (2*dy-1)*a2;
        }
    } while (dy >= 0);

    while (dx++ < a) { /* fehlerhafter Abbruch bei flachen Ellipsen (b=1) */
        oled_set_pixel(x_c+dx, y_c); /* -> Spitze der Ellipse vollenden */
        oled_set_pixel(x_c-dx, y_c);
    }
}

void oled_draw_char(char c, int x0, int y0) {
    uint32_t ret[2];
    uint32_t codePoint = to_cp(&c, ret);
    const uint32_t charDescIndex = codePoint - fontInfo->first_char;

    FontDesc glyph = fontDescs[charDescIndex];
    uint8_t byte;
    uint32_t byte_index;
    uint32_t pitch = (uint32_t) ceil((double)glyph.width / 8);
    int current_bit = 0;

    for (int y = 0; y < glyph.height; ++y) {
        current_bit = 0;

        for (int x = 0; x < glyph.width; ++x) {
            byte_index = (uint32_t) floor((float)x / 8) + glyph.offset;
            byte = *(fontBitmap + y * pitch + byte_index);

            if ((byte & (1 << (7 - current_bit))) >> (7 - current_bit)) {
                if (!(x0 + x >= 128 || y0 + y >= 64)) {
                    oled_set_pixel(x0 + x, y0 + y);
                }
            }

            current_bit += 1;

            if (current_bit ==  (uint8_t) fmin((double)8, (double)(glyph.width - byte_index * 8))) {
                current_bit = 0;
            }
        }
    }
}

void oled_draw_text(char * text, int x0, int y0, bool wrap) {
    uint32_t text_dim[3];

    get_text_dim(text, text_dim);

    FontDesc glyph;

    uint8_t x = 0;
    uint8_t y;
    uint32_t text_len = strlen((char *)text);
    uint32_t baseline = text_dim[2];
    uint32_t height = text_dim[1];
    uint32_t charDescIndex;
    int32_t codePoint;
    uint32_t offset_y = 0;
    uint32_t ret[2];

    for (int i = 0; i < text_len; ++i) {
        codePoint = to_cp(&text[i], ret);
        charDescIndex = codePoint - fontInfo->first_char;
        glyph = fontDescs[charDescIndex];

        if ( x + glyph.width >= 128 ) {
            offset_y += height;
            x = 0;
        }

        y = height - glyph.ascent - (baseline - y0) + offset_y;

        oled_draw_char(text[i], x0 + x, y);

        x += glyph.advance_width;

        i += (ret[0] - 1);
    }
}

void oled_init() {
    i2c_init();
    // Init sequence
    oled_cmd(0xAE); //display off
    oled_cmd(0xA8); // Set Multiplex
    oled_cmd(0x3F);
    oled_cmd(0xD3); // Set Display Offset
    oled_cmd(0x00);
    oled_cmd(0x20); // Set Memory Addressing Mode
    oled_cmd(0x00);
    oled_cmd(0x40); // Set Display Start Line
    oled_cmd(0xA1); // Set Segment re-map 0 to 127
    oled_cmd(0xC8); // Set COM Output Scan Direction
    oled_cmd(0xDA); // Set COM Pins Hardware Configuration
    oled_cmd(0x12);
    oled_cmd(0x81); // Set Contrast Control
    oled_cmd(0x5d);
    oled_cmd(0xA4); // Disable Entire Display On
    oled_cmd(0xA6); // Normal Display
    oled_cmd(0xD5); // Set OSC Freq
    oled_cmd(0xE0);
    oled_cmd(0x8D); // Enable charge pump regulator
    oled_cmd(0x14);
    oled_cmd(0xAF); // Display ON

    // Set column and page address
    oled_cmd(0x21);
    oled_cmd(0x00);
    oled_cmd(127);
    oled_cmd(0x22);
    oled_cmd(0x00);
    oled_cmd(7);

    memset(SSD1306_Buffer, 0x00, sizeof(SSD1306_Buffer));
    oled_cmd(0xB0 + 0);

    for (uint8_t m = 0; m < 8; m += 1) {
        oled_data(
            &SSD1306_Buffer[m * 128],
            128
        );
    }
}

void oled_clear() {
    memset(SSD1306_Buffer, 0x00, sizeof(SSD1306_Buffer));
}

void oled_display() {
    for (int m = 0; m < 8; m += 1) {
        // Write data
        oled_data(
            &SSD1306_Buffer[m * 128],
            128
        );
    }
}

void oled_display2() {
    // Write data
    for (int m = 0; m < 8; m += 4) {
        // Write data
        oled_data(
            &SSD1306_Buffer[m * 128],
            128 * 4
        );
    }
}