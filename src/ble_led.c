#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include "ble_led.h"

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED_B	DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN_B	DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS_B	DT_GPIO_FLAGS(LED0_NODE, gpios)
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led0 devicetree alias is not defined"
#define LED_B	""
#define PIN_B	0
#define FLAGS_B	0
#endif

#if DT_NODE_HAS_STATUS(LED1_NODE, okay)
#define LED_G	DT_GPIO_LABEL(LED1_NODE, gpios)
#define PIN_G	DT_GPIO_PIN(LED1_NODE, gpios)
#define FLAGS_G	DT_GPIO_FLAGS(LED1_NODE, gpios)
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led1 devicetree alias is not defined"
#define LED_G	""
#define PIN_G	0
#define FLAGS_G	0
#endif

#if DT_NODE_HAS_STATUS(LED2_NODE, okay)
#define LED_R	DT_GPIO_LABEL(LED2_NODE, gpios)
#define PIN_R	DT_GPIO_PIN(LED2_NODE, gpios)
#define FLAGS_R	DT_GPIO_FLAGS(LED2_NODE, gpios)
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led2 devicetree alias is not defined"
#define LED_R	""
#define PIN_R	0
#define FLAGS_R	0
#endif

const struct device *b_led;
const struct device *g_led;
const struct device *r_led;

int config_leds(void) 
{
    int ret;
    b_led = device_get_binding(LED_B);
    g_led = device_get_binding(LED_G);
    r_led = device_get_binding(LED_R);
    if ((b_led || g_led || r_led ) == NULL) {
        return -1;
    }

    ret = gpio_pin_configure(b_led, PIN_B, GPIO_OUTPUT_ACTIVE | FLAGS_B);
    ret = gpio_pin_configure(g_led, PIN_G, GPIO_OUTPUT_ACTIVE | FLAGS_G);
    ret = gpio_pin_configure(r_led, PIN_R, GPIO_OUTPUT_ACTIVE | FLAGS_R);
    if (ret < 0) {
        return -2;
    }

    return 0;
}

void set_led(bool to, RGB_t color) 
{

    switch (color) {
        case (BLUE):
            gpio_pin_set(b_led, PIN_B, (int)!to);
            break;
        case (GREEN):
            gpio_pin_set(g_led, PIN_G, (int)!to);
            break;
        case (RED):
            gpio_pin_set(r_led, PIN_R, (int)!to);
            break;
    }

}