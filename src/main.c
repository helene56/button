#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define SLEEP_TIME_MS 100

#define ROW1_NODE DT_ALIAS(row1)
#define ROW2_NODE DT_ALIAS(row2)
#define ROW3_NODE DT_ALIAS(row3)

#define COLA_NODE DT_ALIAS(cola)
#define COLB_NODE DT_ALIAS(colb)
#define COLC_NODE DT_ALIAS(colc)

#if !DT_NODE_HAS_STATUS_OKAY(ROW1_NODE)
#error "row1 alias is not defined"
#endif

#if !DT_NODE_HAS_STATUS_OKAY(ROW2_NODE)
#error "row2 alias is not defined"
#endif

#if !DT_NODE_HAS_STATUS_OKAY(ROW3_NODE)
#error "row3 alias is not defined"
#endif

#if !DT_NODE_HAS_STATUS_OKAY(COLA_NODE)
#error "cola alias is not defined"
#endif

#if !DT_NODE_HAS_STATUS_OKAY(COLB_NODE)
#error "colb alias is not defined"
#endif

#if !DT_NODE_HAS_STATUS_OKAY(COLC_NODE)
#error "colc alias is not defined"
#endif

/*
 * The led0 devicetree alias is optional. If present, we'll use it
 * to turn on the LED whenever the button is pressed.
 */
static struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios,
						     {0});


static const struct gpio_dt_spec row1 = GPIO_DT_SPEC_GET(ROW1_NODE, gpios);
static const struct gpio_dt_spec row2 = GPIO_DT_SPEC_GET(ROW2_NODE, gpios);
static const struct gpio_dt_spec row3 = GPIO_DT_SPEC_GET(ROW3_NODE, gpios);

static const struct gpio_dt_spec cola = GPIO_DT_SPEC_GET(COLA_NODE, gpios);
static const struct gpio_dt_spec colb = GPIO_DT_SPEC_GET(COLB_NODE, gpios);
static const struct gpio_dt_spec colc = GPIO_DT_SPEC_GET(COLC_NODE, gpios);

int main(void)
{
	int ret;

	if (!gpio_is_ready_dt(&row1) || !gpio_is_ready_dt(&row2) || !gpio_is_ready_dt(&row3) ||
	    !gpio_is_ready_dt(&cola) || !gpio_is_ready_dt(&colb) || !gpio_is_ready_dt(&colc)) {
		printk("One or more GPIO devices are not ready\n");
		return 0;
	}

	/* Rows: input + pull-up from devicetree */
	ret = gpio_pin_configure_dt(&row1, GPIO_INPUT);
	if (ret < 0) {
		printk("Failed to configure row1: %d\n", ret);
		return 0;
	}

	ret = gpio_pin_configure_dt(&row2, GPIO_INPUT);
	if (ret < 0) {
		printk("Failed to configure row2: %d\n", ret);
		return 0;
	}

	ret = gpio_pin_configure_dt(&row3, GPIO_INPUT);
	if (ret < 0) {
		printk("Failed to configure row3: %d\n", ret);
		return 0;
	}

	/* Columns: active-low outputs, drive active (physical low). */
	ret = gpio_pin_configure_dt(&cola, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Failed to configure cola: %d\n", ret);
		return 0;
	}

	ret = gpio_pin_configure_dt(&colb, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Failed to configure colb: %d\n", ret);
		return 0;
	}

	ret = gpio_pin_configure_dt(&colc, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Failed to configure colc: %d\n", ret);
		return 0;
	}

	if (led.port) {
		ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
		if (ret != 0) {
			printk("Error %d: failed to configure LED device %s pin %d\n",
			       ret, led.port->name, led.pin);
			led.port = NULL;
		} else {
			printk("Set up LED at %s pin %d\n", led.port->name, led.pin);
		}
	}
	

	printk("Rows configured as pull-up inputs, columns active-low and driven low\n");

	while (1) {
		int r1 = gpio_pin_get_dt(&row1);
		int r2 = gpio_pin_get_dt(&row2);
		int r3 = gpio_pin_get_dt(&row3);

		printk("row1=%d row2=%d row3=%d\n", r1, r2, r3);
		if (r1 || r2 || r3)
		{
			gpio_pin_set_dt(&led, 1);
			// if (val >= 0) {
			// 	gpio_pin_set_dt(&led, val);
			// }
		}
		else
		{
			gpio_pin_set_dt(&led, 0);
		}
		k_msleep(SLEEP_TIME_MS);
	}

	return 0;
}
