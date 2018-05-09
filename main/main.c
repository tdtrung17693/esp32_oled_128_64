#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "stdio.h"
#include "string.h"
#include "esp_log.h"
#include "esp_system.h"
#include "oled_lib.h"
#include "bitmap.h"

#include "unifont15pt.h"
#include "ledDisplay50pt.h"

#define PI 3.14159265358979323846264338327950

void oled_draw_sin_wave(float angle, float coef, float x0, float y0) {

    oled_set_pixel((int)round(angle + x0), (int)round(15 * sin(coef * angle) + y0));
    oled_set_pixel((int)round(angle + x0), (int)round(-15 * sin(coef * angle) + y0));

}

void oled_draw_parabolic_curve(float point_amount, float coef) {
    // Parabolic
    for (float i = 0; i <= point_amount; i += 0.01 ) {
        oled_set_pixel((int)(i + 63.0), (int)(coef * i * i));
        oled_set_pixel((int)(-i + 63.0), (int)(coef * i * i));
    }
}

void oled_draw_butterfly_curve(float angle) {
    // Butterfly Curve
    oled_set_pixel(
        (int)round(7 * sin(angle) * (exp(cos(angle)) - 2 * cos(4 * angle) - pow(sin(angle/12), 5)) + 63.0),
        (int)round(-7 * cos(angle) * (exp(cos(angle)) - 2 * cos(4 * angle) - pow(sin(angle/12), 5)) + 31.0)
    );
}

void oled_count_text() {
    static int count = 0;
    static int dir = 1;
    char str[4];

    sprintf(str, "%d", count);
    oled_set_font(fontled_display750ptDescriptors, &fontled_display750ptInfo, fontled_display750pt);
    oled_draw_text(str, 10, 10, false);

    count += dir * 1;

    if (count == 9999 || count == 0) dir = -dir;
}

void oled_display_task(void *params) {
    oled_clear();

    float coef = 0.25;
    int dir = 1;
    float r = 4.0;
    float x0 = 90;

    ESP_LOGI("OLED", "%c", L'ü');

    while (1) {
        oled_clear();

        for (float i = 0; i <= 50*PI; i += PI/18) {
            oled_draw_sin_wave(i, coef, 0, 20.0);

        }

        if (coef <= 0.01 || coef >= 0.5) {
            dir = -dir;
        }

        coef += dir * 0.01;
        r += dir * 0.2;
        x0 += dir;


        oled_display();
    }
}

void app_main(void) {
    esp_timer_init();
    oled_init();

    xTaskCreate(oled_display_task, "OLED Display Task", 2048, NULL, 2, NULL);
}