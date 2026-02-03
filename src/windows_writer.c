#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>
#include "windows_writer.h"

// Helper to run commands
static int run_cmd(const char *fmt, ...) {
    char *cmd = NULL;
    va_list ap;
    va_start(ap, fmt);
    if (vasprintf(&cmd, fmt, ap) == -1) {
        va_end(ap);
        return -1;
    }
    va_end(ap);

    printf("Executing: %s\n", cmd);
    int ret = system(cmd);
    free(cmd);
    return ret == 0 ? 0 : -1;
}

int is_windows_iso(const char *iso_path) {
    // Simple check: Look for setup.exe or sources/install.wim in the ISO
    // Using isoinfo or 7z would be better, but we'll try a simpler check using grep on the file usually works for check
    // Actually, shelling out to `osirrox` (xorriso) or just mounting it strictly for check is better.
    // Let's assume if it is NOT hybrid, it might be Windows.
    // For now, simpler: user told us.
    // A robust check: mount it loopback and check content.
    
    char mount_point[] = "/tmp/linrufus_iso_check_XXXXXX";
    if (!mkdtemp(mount_point)) return 0;
    
    int is_win = 0;
    run_cmd("mount -o loop,ro \"%s\" %s >/dev/null 2>&1", iso_path, mount_point);
    
    struct stat st;
    char path[1024];
    snprintf(path, sizeof(path), "%s/setup.exe", mount_point);
    if (stat(path, &st) == 0) is_win = 1;
    else {
        snprintf(path, sizeof(path), "%s/sources/install.wim", mount_point); // Case sensitivity? ISO9660 is usually caps/nocaps handled by mount options
        if (stat(path, &st) == 0) is_win = 1;
    }
    
    run_cmd("umount %s >/dev/null 2>&1", mount_point);
    rmdir(mount_point);
    
    return is_win;
}

int write_windows_iso(const char *iso_path, const char *device_path) {
    printf("Windows ISO detected. Switching to FAT32 + WIM Split mode.\n");
    
    // 0. Unmount
    run_cmd("umount %s* >/dev/null 2>&1", device_path);

    // 1. Wipe MBR/GPT
    printf("Wiping MBR/GPT...\n");
    if (run_cmd("parted -s %s mklabel gpt", device_path) != 0) return -1;

    // 2. Create FAT32 partition
    printf("Creating FAT32 Partition...\n");
    if (run_cmd("parted -s %s mkpart primary fat32 1MB 100%%", device_path) != 0) return -1;
    if (run_cmd("parted -s %s set 1 msftdata on", device_path) != 0) return -1;
    
    run_cmd("partprobe %s", device_path);
    sleep(1);

    char part_path[128];
    if (device_path[strlen(device_path)-1] >= '0' && device_path[strlen(device_path)-1] <= '9')
         snprintf(part_path, sizeof(part_path), "%sp1", device_path);
    else
         snprintf(part_path, sizeof(part_path), "%s1", device_path);

    // 3. Format FAT32
    printf("Formatting FAT32...\n");
    if (run_cmd("mkfs.vfat -F 32 -n LinRufus %s", part_path) != 0) return -1;

    // 4. Mount
    char iso_mount[] = "/tmp/linrufus_iso_XXXXXX";
    char dest_mount[] = "/tmp/linrufus_dest_XXXXXX";
    if (!mkdtemp(iso_mount) || !mkdtemp(dest_mount)) return -1;

    int ret = 0;
    if (run_cmd("mount -o loop,ro \"%s\" %s", iso_path, iso_mount) != 0) { ret = -1; goto cleanup; }
    if (run_cmd("mount %s %s", part_path, dest_mount) != 0) { ret = -1; goto cleanup; }

    // 5. Copy Files (excluding install.wim)
    printf("Copying files (excluding install.wim)...\n");
    // Use rsync to exclude or manual cp. 
    // Simple way: copy everything then rm install.wim and run split? 
    // NO, install.wim > 4GB will fail to copy to FAT32.
    // So we must selectively copy.
    // rsync is good: rsync -av --exclude 'install.wim' /mnt/iso/ /mnt/usb/
    if (run_cmd("rsync -rv --no-o --no-g --exclude 'install.wim' --exclude 'sources/install.wim' %s/ %s/", iso_mount, dest_mount) != 0) {
        // Fallback if rsync not present? But we should have it.
        printf("Rsync failed. Do you have rsync installed?\n");
        ret = -1; 
        goto cleanup;
    }

    // 6. Split WIM
    printf("Splitting install.wim (>4GB) to install.swm...\n");
    // Check where install.wim is (usually sources/install.wim)
    char wim_src[1024];
    snprintf(wim_src, sizeof(wim_src), "%s/sources/install.wim", iso_mount);
    
    // Verify it exists, some ISOs might use install.esd (which is usually <4GB compressed, so rsync might have copied it if we didn't exclude. But wait, install.esd can be huge too? FAT32 limit applies).
    // If install.wim doesn't exist, maybe it was install.esd?
    struct stat st;
    if (stat(wim_src, &st) != 0) {
        // Try esd
        snprintf(wim_src, sizeof(wim_src), "%s/sources/install.esd", iso_mount);
        if (stat(wim_src, &st) != 0) {
            // Neither found? Maybe x86? Or custom layout.
            // If rsync copied everything else, maybe it's fine.
            printf("Warning: install.wim/esd not found in standard location.\n");
        } else {
             // It is ESD.
             // If ESD > 4GB, we need to split it too? wimlib supports splitting ESD?
             // Actually, usually ESD is used specifically to squeeze under 4GB.
             // If it's already copied (rsync didn't exclude esd), then check if it failed?
             // Rsync would fail with "File too large".
             // We should check size.
             if (st.st_size > 4294967295L) {
                 printf("Large ESD detected. Attempting split...\n");
                 run_cmd("wimlib-imagex split \"%s\" \"%s/sources/install.swm\" 4000", wim_src, dest_mount);
             } else {
                 // It fits. Rsync should have copied it if we didn't exclude it.
                 // But we excluded 'sources/install.wim'. We didn't exclude esd.
                 // So if it was ESD, it should be there.
             }
        }
    } else {
        // WIM Found.
        if (stat(wim_src, &st) == 0 && st.st_size > 4294967295L) {
            run_cmd("wimlib-imagex split \"%s\" \"%s/sources/install.swm\" 4000", wim_src, dest_mount);
        } else {
            // It fits. Copy normally if not split.
            run_cmd("cp \"%s\" \"%s/sources/install.wim\"", wim_src, dest_mount);
        }
    }

    printf("Bootloader setup... (Standard UEFI FAT32 layout used)\n");
    // No grub-install needed for Windows 8/10/11 on UEFI! 
    // They look for /efi/boot/bootx64.efi which we copied from ISO.

cleanup:
    run_cmd("umount %s", dest_mount);
    run_cmd("umount %s", iso_mount);
    rmdir(dest_mount);
    rmdir(iso_mount);
    return ret;
}
