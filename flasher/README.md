# Web Flasher

This directory contains the ESP Web Tools-based firmware flasher for the 7-Segment LED Clock.

## Directory Structure

```
flasher/
├── index.html          # Web flasher page (hosted on GitHub Pages)
├── manifest.json       # ESP Web Tools manifest
├── README.md           # This file
└── fw/
    └── esp32/          # Firmware binaries for ESP32
        ├── bootloader.bin
        ├── partitions.bin
        └── application.bin
```

## Updating Firmware Files

Before each release, copy the firmware files from the PlatformIO build output:

```bash
# From the project root directory
cp .pio/build/esp32dev/bootloader.bin flasher/fw/esp32/
cp .pio/build/esp32dev/partitions.bin flasher/fw/esp32/
cp .pio/build/esp32dev/firmware.bin flasher/fw/esp32/application.bin
```

**Note:** The main firmware file is renamed from `firmware.bin` to `application.bin` to match the manifest.

## GitHub Pages Deployment

The flasher is hosted via GitHub Pages. To enable:

1. Go to repository **Settings** → **Pages**
1. Under "Source", select **Deploy from a branch**
1. Select branch `main` and folder `/flasher`
1. Click **Save**

The flasher will be available at: `https://ursweiss.github.io/7-Segment-LED-Clock/`

## Manifest Configuration

The `manifest.json` file defines the firmware parts and their flash offsets:

| File            | Offset          | Description      |
| --------------- | --------------- | ---------------- |
| bootloader.bin  | 0x1000 (4096)   | ESP32 bootloader |
| partitions.bin  | 0x8000 (32768)  | Partition table  |
| application.bin | 0x10000 (65536) | Main firmware    |

## Testing Locally

To test the flasher locally before pushing:

```bash
# From the flasher directory
python3 -m http.server 8000
```

Then open `http://localhost:8000` in Chrome or Edge.

**Note:** Web Serial requires HTTPS in production, but works on localhost for testing.
