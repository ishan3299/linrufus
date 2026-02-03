#ifndef LINRUFUS_WINDOWS_WRITER_H
#define LINRUFUS_WINDOWS_WRITER_H

// Check if an ISO seems to be a Windows ISO
// Returns 1 if yes, 0 if no
int is_windows_iso(const char *iso_path);

// Write Windows ISO to device using partitioning/formatting/copying
// This DESTROYS data on the device.
// Returns 0 on success, < 0 on error
int write_windows_iso(const char *iso_path, const char *device_path);

#endif // LINRUFUS_WINDOWS_WRITER_H
