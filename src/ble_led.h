typedef enum {
    BLUE, GREEN, RED
} RGB_t;
int config_leds(void);
void set_led(bool, RGB_t);