#!/bin/bash

# update the serial udev
if [ ! -f /dev/serial/by-id/usb-Infineon_IFX_CDC-if00 ]; then
    echo "create link /dev/serial/by-id/usb-Infineon_IFX_CDC-if00 -> /dev/ttyACM0..."
    mkdir -p /dev/serial/by-id
    ln -s /dev/ttyACM0 /dev/serial/by-id/usb-Infineon_IFX_CDC-if00
fi