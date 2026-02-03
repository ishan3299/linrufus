#ifndef LINRUFUS_WRITER_H
#define LINRUFUS_WRITER_H

#include <stddef.h>

// Progress callback function type
// written: bytes written so far
// total: total bytes to write
typedef void (*progress_cb_t)(size_t written, size_t total);

// Write an image file to a device
// image_path: path to source ISO/IMG
// device_path: path to target block device
// callback: update function (can be NULL)
// Returns 0 on success, < 0 on error
int write_image_to_device(const char *image_path, const char *device_path, progress_cb_t callback);

#endif // LINRUFUS_WRITER_H
