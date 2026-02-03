#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "device.h"
#include "writer.h"

#define VERSION "0.1.0"

void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS]\n", prog_name);
    printf("Options:\n");
    printf("  -h, --help       Show this help message\n");
    printf("  -v, --version    Show version\n");
    printf("  -l, --list       List available USB devices\n");
    printf("  -d, --device     Target USB device (e.g., /dev/sdb)\n");
    printf("  -i, --image      Source ISO image path\n");
}

void progress_bar(size_t written, size_t total) {
    const int bar_width = 50;
    float progress = (float)written / total;
    int pos = (int)(bar_width * progress);

    printf("\r[");
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) printf("=");
        else if (i == pos) printf(">");
        else printf(" ");
    }
    printf("] %.1f%%", progress * 100.0);
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    int opt;
    int option_index = 0;
    char *device_path = NULL;
    char *image_path = NULL;
    
    static struct option long_options[] = {
        {"help",    no_argument,       0, 'h'},
        {"version", no_argument,       0, 'v'},
        {"list",    no_argument,       0, 'l'},
        {"device",  required_argument, 0, 'd'},
        {"image",   required_argument, 0, 'i'},
        {0, 0, 0, 0}
    };

    if (argc == 1) {
        print_usage(argv[0]);
        return EXIT_SUCCESS;
    }

    while ((opt = getopt_long(argc, argv, "hvld:i:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                print_usage(argv[0]);
                return EXIT_SUCCESS;
            case 'v':
                printf("linrufus version %s\n", VERSION);
                return EXIT_SUCCESS;
            case 'l': {
                usb_device_t devices[32];
                int count = get_usb_devices(devices, 32);
                if (count < 0) {
                    fprintf(stderr, "Error listing devices\n");
                    return EXIT_FAILURE;
                }
                if (count == 0) {
                    printf("No USB devices found.\n");
                } else {
                    printf("Found %d device(s):\n", count);
                    for (int i = 0; i < count; i++) {
                        print_device_info(&devices[i]);
                    }
                }
                goto cleanup; 
            }
            case 'd':
                device_path = optarg;
                break;
            case 'i':
                image_path = optarg;
                break;
            case '?':
                return EXIT_FAILURE;
            default:
                break;
        }
    }

    if (device_path && image_path) {
        printf("Writing image '%s' to device '%s'...\n", image_path, device_path);
        // Warning
        printf("WARNING: ALL DATA ON %s WILL BE DESTROYED.\n", device_path);
        
        if (write_image_to_device(image_path, device_path, progress_bar) == 0) {
            printf("\nSuccess!\n");
        } else {
            fprintf(stderr, "\nFailed to write image.\n");
            return EXIT_FAILURE;
        }
    } else if (device_path || image_path) {
        fprintf(stderr, "Error: Both device (-d) and image (-i) must be specified.\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

cleanup:
    return EXIT_SUCCESS;
}
