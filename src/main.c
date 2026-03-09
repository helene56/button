#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define KBD_NODE DT_ALIAS(keyboard)

#if !DT_NODE_HAS_STATUS_OKAY(KBD_NODE)
#error "keyboard alias is not defined"
#endif

#define KBD_DEV DEVICE_DT_GET(KBD_NODE)

static struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});

static void kbd_cb(struct input_event *evt, void *user_data)
{
	static int row;
	static int col;
	static int pressed;

	ARG_UNUSED(user_data);

	switch (evt->code) {
	case INPUT_ABS_X:
		col = evt->value;
		break;
	case INPUT_ABS_Y:
		row = evt->value;
		break;
	case INPUT_BTN_TOUCH:
		pressed = evt->value;
		break;
	default:
		return;
	}

	if (!evt->sync) {
		return;
	}

	printk("row=%d col=%d %s\n", row, col, pressed ? "pressed" : "released");

	if (pressed && led.port != NULL) {
		gpio_pin_toggle_dt(&led);
	}
}

INPUT_CALLBACK_DEFINE(KBD_DEV, kbd_cb, NULL);

int main(void)
{
	const struct device *kbd = KBD_DEV;
	int ret;

	if (!device_is_ready(kbd)) {
		printk("Keyboard device %s is not ready\n", kbd->name);
		return 0;
	}

	if (led.port != NULL) {
		if (!gpio_is_ready_dt(&led)) {
			printk("LED GPIO device is not ready\n");
			led.port = NULL;
		} else {
			ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
			if (ret != 0) {
				printk("Failed to configure LED: %d\n", ret);
				led.port = NULL;
			}
		}
	}

	printk("Keyboard matrix input started on %s\n", kbd->name);

	while (1) {
		k_msleep(1000);
	}

	return 0;
}
