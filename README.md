# LinRufus (Linux-native Rufus)

**LinRufus** is a Linux port of the popular [Rufus](https://rufus.ie) USB formatting utility.
While Rufus is originally a Windows-only tool, LinRufus aims to bring the same reliability and ease of use to Linux users via a command-line interface (CLI) and native Linux APIs.

> [!WARNING]
> This is a **Linux-specific fork**. Please do not report issues encountered in LinRufus to the upstream Rufus repository unless you are certain the issue exists there as well.

## Features (Planned)
- **CLI Only**: Simple, scriptable command-line interface.
- **Native Backend**: Uses `libudev` and standard Linux block device operations.
- **Safe**: Includes checks to prevent overwriting system drives.
- **Debian Packaging**: Easy installation via `.deb` packages.

## Installation
### From Source
```bash
git clone https://github.com/ishan3299/linrufus
cd linrufus
make
sudo make install
```

### Debian/Ubuntu
```bash
sudo dpkg -i linrufus_*.deb
```

## Usage
```bash
# List available USB devices
linrufus --list

# Write an ISO to a device
sudo linrufus --device /dev/sdb --image ./ubuntu.iso
```

## Attribution
This project is based on [Rufus](https://github.com/pbatard/rufus) by Pete Batard.
Copyright © 2011-2024 Pete Batard <pete@akeo.ie>
LinRufus modifications Copyright © 2024 Ishan Patel <ishan.patel1998@gmail.com>

## License
GPLv3 - See [LICENSE](LICENSE) for details.
