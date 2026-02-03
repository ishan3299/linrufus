#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libudev.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include "device.h"

int get_usb_devices(usb_device_t *devices, int max_devices) {
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices_list, *dev_list_entry;
    struct udev_device *dev;
    int count = 0;

    udev = udev_new();
    if (!udev) {
        fprintf(stderr, "Cannot create udev context.\n");
        return -1;
    }

    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_add_match_property(enumerate, "ID_BUS", "usb");
    udev_enumerate_scan_devices(enumerate);

    devices_list = udev_enumerate_get_list_entry(enumerate);
    udev_list_entry_foreach(dev_list_entry, devices_list) {
        if (count >= max_devices) break;

        const char *path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        // We only want the main disk, not partitions (e.g. sdb, not sdb1)
        const char *devtype = udev_device_get_devtype(dev);
        if (!devtype || strcmp(devtype, "disk") != 0) {
            udev_device_unref(dev);
            continue;
        }

        const char *devnode = udev_device_get_devnode(dev);
        if (devnode) {
            strncpy(devices[count].path, devnode, sizeof(devices[count].path) - 1);
            
            const char *vendor = udev_device_get_property_value(dev, "ID_VENDOR");
            const char *model = udev_device_get_property_value(dev, "ID_MODEL");
            
            if (vendor) strncpy(devices[count].vendor, vendor, sizeof(devices[count].vendor) - 1);
            else strncpy(devices[count].vendor, "Unknown", sizeof(devices[count].vendor) - 1);
            
            if (model) strncpy(devices[count].model, model, sizeof(devices[count].model) - 1);
            else strncpy(devices[count].model, "Unknown", sizeof(devices[count].model) - 1);

            // Get attributes for size
            const char *sysattr_size = udev_device_get_sysattr_value(dev, "size"); // in 512-byte sectors
            if (sysattr_size) {
                uint64_t sectors = strtoull(sysattr_size, NULL, 10);
                devices[count].size_bytes = sectors * 512;
            } else {
                devices[count].size_bytes = 0;
            }
            
            devices[count].is_usb = 1; // Filter managed that
            devices[count].is_removable = 1; // Assumption for USB mass storage

            count++;
        }

        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return count;
}

void print_device_info(const usb_device_t *device) {
    double size_gb = (double)device->size_bytes / (1024 * 1024 * 1024);
    printf("%s: %s %s (%.2f GB)\n", device->path, device->vendor, device->model, size_gb);
}
