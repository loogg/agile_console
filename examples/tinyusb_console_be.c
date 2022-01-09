#include <stdint.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <agile_console.h>
#include <tusb.h>

static struct agile_console_backend _console_backend = {0};
static struct rt_mutex _mtx;
static struct rt_timer _timer;
static uint8_t _shield_flag = 0;

static void console_backend_output(rt_device_t dev, const uint8_t *buf, int len)
{
    if (_shield_flag)
        return;

    if (rt_interrupt_get_nest())
        return;

    if (rt_critical_level())
        return;

    if (!tud_cdc_connected())
        return;

    rt_mutex_take(&_mtx, RT_WAITING_FOREVER);

    if (dev->open_flag & RT_DEVICE_FLAG_STREAM) {
        while (len > 0) {
            if (*buf == '\n')
                tud_cdc_write_char('\r');

            tud_cdc_write_char((char)(*buf));

            ++buf;
            --len;
        }
    } else
        tud_cdc_write(buf, len);

    tud_cdc_write_flush();

    rt_mutex_release(&_mtx);
}

static int console_backend_read(rt_device_t dev, uint8_t *buf, int len)
{
    if (_shield_flag)
        return 0;

    if (!tud_cdc_connected())
        return 0;

    if (!tud_cdc_available())
        return 0;

    return tud_cdc_read(buf, len);
}

static void timer_timeout(void *parameter)
{
    if (_shield_flag)
        return;

    if (tud_cdc_available())
        agile_console_wakeup();
}

static int tinyusb_console_init(void)
{
    rt_mutex_init(&_mtx, "con_usb", RT_IPC_FLAG_PRIO);
    rt_timer_init(&_timer, "con_usb", timer_timeout, RT_NULL, 10, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_PERIODIC);
    rt_timer_start(&_timer);

    _console_backend.output = console_backend_output;
    _console_backend.read = console_backend_read;
    agile_console_backend_register(&_console_backend);

    return RT_EOK;
}
INIT_ENV_EXPORT(tinyusb_console_init);

static int tinyusb_console_disable(void)
{
    _shield_flag = 1;

    return RT_EOK;
}
MSH_CMD_EXPORT(tinyusb_console_disable, disable tinyusb console);

static int tinyusb_console_enable(void)
{
    _shield_flag = 0;
    tud_cdc_read_flush();

    return RT_EOK;
}
MSH_CMD_EXPORT(tinyusb_console_enable, enable tinyusb console);
