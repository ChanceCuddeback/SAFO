#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

#define LED0_NODE DT_ALIAS(ledb)

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

const struct device *led;

int config_led(void) 
{
    int ret;
    led = device_get_binding(LED_B);
    if (led == NULL) {
        return 1;
    }

    ret = gpio_pin_configure(led, PIN_B, GPIO_OUTPUT_ACTIVE | FLAGS_B);
    if (ret < 0) {
        return 11;
    }

    return 0;
}

void set_led(bool to) 
{
    /* Invert the input because the led is active low */
    gpio_pin_set(led, PIN_B, (int)!to);
}