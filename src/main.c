#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define SLEEP_TIME_MS 10

#define STACKSIZE 1024
#define THREAD0_PRIORITY 7
#define THREAD1_PRIORITY 7

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

// static struct gpio_callback button_cb_data;
// static struct k_work_delayable row1_debounce_work;
static volatile bool row1_debounce_pending;
static volatile bool row1_pressed;

#define ROW1_DEBOUNCE_MS 15

static struct gpio_dt_spec col_arr[] = {cola, colb, colc};
int active_col = 0;
static volatile uint8_t col_state[3] = {0, 0, 0};
static volatile uint8_t row_mask = 0;
uint8_t last_state = 0;
// col scan
void scan_btn_matrix()
{
    while (1)
    {
        // printk("Hello, I am thread0\n");
        // should set it to inactive
        gpio_pin_set_dt(&cola, 0);
        gpio_pin_set_dt(&colb, 0);
        gpio_pin_set_dt(&colc, 0);

        // set one to true
        gpio_pin_set_dt(&col_arr[active_col], 1);
        k_busy_wait(30);
        // rows
        row_mask = ((1 ? gpio_pin_get_dt(&row3) : 0) << 2) | 
                   ((1 ? gpio_pin_get_dt(&row2) : 0) << 1) |
                   ((1 ? gpio_pin_get_dt(&row1) : 0) << 0);
        
        // update states
        col_state[active_col] = row_mask;
        
        // advance column
        active_col++;
        if (active_col > 2)
            active_col = 0;

        k_msleep(1);
    }
}


void read_rows()
{
    while(1)
    {
        uint8_t matrix_mask = col_state[0] | col_state[1] | col_state[2];
        bool any = (matrix_mask & 0b0111) != 0;
        static bool last_any = false;
        if (any && !last_any)
        {
            for (int r = 0; r < 3; r++)
            {
                for (int c = 0; c < 3; c++)
                {
                    bool rc = (col_state[c] & (1u << r)) != 0;
                    
                    if (rc)
                    {
                        // if (last_state == (col_state[c] & (1u << r)))
                        // {
                        //     count++;
                        // }
                        // if (count>10)
                        // {
                        //     printk("r: %d, c: %d pressed\n", r, c);
                        //     gpio_pin_toggle_dt(&led);
                        //     count = 0;
                        // }
                        // last_state = (col_state[c] & (1u << r));
                        printk("r: %d, c: %d pressed\n", r, c);
                        gpio_pin_toggle_dt(&led);


                        
                        
                    }
                        
                }
            }
        }
        last_any = any;
        k_msleep(15);
    }
    
}

// // define callback function for interrupt on 'button'
// void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
// {
//     ARG_UNUSED(dev);
//     ARG_UNUSED(cb);
//     ARG_UNUSED(pins);

//     if (row1_debounce_pending)
//     {
//         return;
//     }

//     row1_debounce_pending = true;
//     k_work_schedule(&row1_debounce_work, K_MSEC(ROW1_DEBOUNCE_MS));
// }

// static void row1_debounce_handler(struct k_work *work)
// {
//     // int pressed;

//     ARG_UNUSED(work);

//     // pressed = gpio_pin_get_dt(&row1);
//     // if (pressed > 0)
//     // {

//     //     if (!row1_pressed)
//     //     {

//     //         row1_pressed = true;
//     //         gpio_pin_toggle_dt(&led);
//     //     }
//     // }
//     // else if (pressed == 0)
//     // {
//     //     row1_pressed = false;
//     // }
//     uint8_t matrix_mask = col_state[0] | col_state[1] | col_state[2];
//     bool any = (matrix_mask & 0b0111) != 0;
//     static int count = 0;
//     if (any)
//     {
        
//         for (int r = 0; r < 3; r++)
//         {
//             for (int c = 0; c < 3; c++)
//             {
//                 bool rc = (col_state[c] & (1u << r)) != 0;
                
//                 if (rc)
//                 {
                    
//                     count++;
//                     if (count > 3 && last_state == (col_state[c] & (1u << r)))
//                     {
//                         count = 0;
//                         break;
//                     }
//                     else
//                     {
//                         printk("r: %d, c: %d pressed\n", r, c);
//                         gpio_pin_toggle_dt(&led);
//                         last_state = (col_state[c] & (1u << r));
//                     }
                    
                    
//                 }
                    
//             }
//         }
//     }


//     row1_debounce_pending = false;
// }

int main(void)
{
    int ret;

    if (!gpio_is_ready_dt(&row1) || !gpio_is_ready_dt(&row2) || !gpio_is_ready_dt(&row3) ||
        !gpio_is_ready_dt(&cola) || !gpio_is_ready_dt(&colb) || !gpio_is_ready_dt(&colc))
    {
        printk("One or more GPIO devices are not ready\n");
        return 0;
    }

    /* Rows: input + pull-up from devicetree */
    ret = gpio_pin_configure_dt(&row1, GPIO_INPUT);
    if (ret < 0)
    {
        printk("Failed to configure row1: %d\n", ret);
        return 0;
    }

    ret = gpio_pin_configure_dt(&row2, GPIO_INPUT);
    if (ret < 0)
    {
        printk("Failed to configure row2: %d\n", ret);
        return 0;
    }

    ret = gpio_pin_configure_dt(&row3, GPIO_INPUT);
    if (ret < 0)
    {
        printk("Failed to configure row3: %d\n", ret);
        return 0;
    }

    /* Columns: active-low outputs, drive active (physical low). */
    ret = gpio_pin_configure_dt(&cola, GPIO_OUTPUT_INACTIVE);
    if (ret < 0)
    {
        printk("Failed to configure cola: %d\n", ret);
        return 0;
    }

    ret = gpio_pin_configure_dt(&colb, GPIO_OUTPUT_INACTIVE);
    if (ret < 0)
    {
        printk("Failed to configure colb: %d\n", ret);
        return 0;
    }

    ret = gpio_pin_configure_dt(&colc, GPIO_OUTPUT_INACTIVE);
    if (ret < 0)
    {
        printk("Failed to configure colc: %d\n", ret);
        return 0;
    }

    if (led.port)
    {
        ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
        if (ret != 0)
        {
            printk("Error %d: failed to configure LED device %s pin %d\n",
                   ret, led.port->name, led.pin);
            led.port = NULL;
        }
        else
        {
            printk("Set up LED at %s pin %d\n", led.port->name, led.pin);
        }
    }

    printk("Rows configured as pull-up inputs, columns active-low and driven low\n");

    // configure interrupts
    // TODO change 'button'
    // associate 'button' with the interrupt
    // k_work_init_delayable(&row1_debounce_work, row1_debounce_handler);
    // ret = gpio_pin_interrupt_configure_dt(&row1, GPIO_INT_EDGE_BOTH);
    // // init callback
    // gpio_init_callback(&button_cb_data, button_pressed, BIT(row1.pin));
    // // add callback
    // gpio_add_callback(row1.port, &button_cb_data);

    while (1)
    {
        // int r1 = gpio_pin_get_dt(&row1);
        // int r2 = gpio_pin_get_dt(&row2);
        // int r3 = gpio_pin_get_dt(&row3);

        // printk("row1=%d row2=%d row3=%d\n", r1, r2, r3);
        // if (r1 || r2 || r3)
        // {
        // 	gpio_pin_set_dt(&led, 1);
        // 	// if (val >= 0) {
        // 	// 	gpio_pin_set_dt(&led, val);
        // 	// }
        // }
        // else
        // {
        // 	gpio_pin_set_dt(&led, 0);
        // }
        k_msleep(SLEEP_TIME_MS);
    }

    return 0;
}

K_THREAD_DEFINE(thread0_id, STACKSIZE, scan_btn_matrix, NULL, NULL, NULL,
                THREAD0_PRIORITY, 0, 0);
K_THREAD_DEFINE(thread1_id, STACKSIZE, read_rows, NULL, NULL, NULL,
                THREAD0_PRIORITY, 0, 0);