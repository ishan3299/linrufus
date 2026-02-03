#ifndef LINRUFUS_DEVICE_H
#define LINRUFUS_DEVICE_H

#include <stdint.h>

typedef struct {
    char path[64];       // e.g., /dev/sdb
    char model[128];     // e.g., SanDisk Ultra
    char vendor[128];
    uint64_t size_bytes;
    int is_removable;
    int is_usb;
} usb_device_t;

// List connected USB devices
// Returns number of devices found
int get_usb_devices(usb_device_t *devices, int max_devices);

// Print device info
void print_device_info(const usb_device_t *device);

#endif // LINRUFUS_DEVICE_H
