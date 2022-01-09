#ifndef __PKG_AGILE_CONSOLE_H
#define __PKG_AGILE_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <rtthread.h>
#include <rtdevice.h>
#include <stdint.h>

struct agile_console_backend {
    void (*output)(rt_device_t dev, const uint8_t *buf, int len);
    int (*read)(rt_device_t dev, uint8_t *buf, int len);
    int (*control)(rt_device_t dev, int cmd, void *arg);
    rt_slist_t slist;
};

struct agile_console {
    struct rt_device parent;
    struct rt_ringbuffer rx_rb;
    struct rt_semaphore rx_notice;
    uint8_t rx_init_ok;
};

int agile_console_backend_register(struct agile_console_backend *backend);
void agile_console_wakeup(void);

#ifdef __cplusplus
}
#endif

#endif
