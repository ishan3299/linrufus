#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include "writer.h"

#define BUFFER_SIZE (1024 * 1024) // 1MB buffer

int write_image_to_device(const char *image_path, const char *device_path, progress_cb_t callback) {
    int img_fd = -1;
    int dev_fd = -1;
    char *buffer = NULL;
    ssize_t bytes_read, bytes_written;
    struct stat img_stat;
    size_t total_written = 0;
    int ret = 0;

    // Open image file
    img_fd = open(image_path, O_RDONLY);
    if (img_fd < 0) {
        perror("Failed to open image file");
        return -1;
    }

    // Get image size
    if (fstat(img_fd, &img_stat) < 0) {
        perror("Failed to get image size");
        close(img_fd);
        return -1;
    }

    // Open target device
    // O_WRONLY | O_SYNC to ensure data is flushed
    dev_fd = open(device_path, O_WRONLY | O_SYNC);
    if (dev_fd < 0) {
        perror("Failed to open target device");
        close(img_fd);
        return -1;
    }

    // Allocate buffer
    buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("Failed to allocate buffer");
        close(img_fd);
        close(dev_fd);
        return -1;
    }

    // Copy loop
    while ((bytes_read = read(img_fd, buffer, BUFFER_SIZE)) > 0) {
        ssize_t written_this_block = 0;
        while (written_this_block < bytes_read) {
            bytes_written = write(dev_fd, buffer + written_this_block, bytes_read - written_this_block);
            if (bytes_written < 0) {
                perror("Failed to write to device");
                ret = -1;
                goto cleanup;
            }
            written_this_block += bytes_written;
            total_written += bytes_written;
        }

        if (callback) {
            callback(total_written, img_stat.st_size);
        }
    }

    if (bytes_read < 0) {
        perror("Failed to read image file");
        ret = -1;
    }

cleanup:
    free(buffer);
    if (dev_fd >= 0) {
        fsync(dev_fd); // Ensure flush
        close(dev_fd);
    }
    if (img_fd >= 0) close(img_fd);

    return ret;
}
