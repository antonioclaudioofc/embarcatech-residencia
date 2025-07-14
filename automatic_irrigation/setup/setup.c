#include "led/led.h"
#include "buzzer/buzzer.h"
#include "display/display.h"
#include "oled/oled.h"
#include "oled/oled_utils.h"
#include "general_config.h"

void setup()
{
    stdio_init_all();

    setup_init_oled();
    oled_clear(oled_buffer, &area);
    render_on_display(oled_buffer, &area);

    setup_led();

    setup_buzzer();

}