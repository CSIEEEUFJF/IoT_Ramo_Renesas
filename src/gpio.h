#ifndef GPIO_H
#define GPIO_H
#include <stdbool.h>

void gpio_init(void);
void gpio_set_door(bool open);
void gpio_set_light(bool on);

#endif /* GPIO_H */
