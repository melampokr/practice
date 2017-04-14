#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <string.h>

#include <libudev.h>

int main(void)
{
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;

    const char *path;

    const char *tmp;

    udev = udev_new();
    if (!udev) {
        printf("Can't create udev\n");
        exit(1);
    }

    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_scan_devices(enumerate);
    devices =udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices) {
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        if (strncmp(udev_device_get_devtype(dev), "partition", 9) == 0 ||
            strncmp(udev_device_get_sysname(dev), "loop", 4) == 0 ||
            strncmp(udev_device_get_sysname(dev), "sr", 2) == 0 ||
            strncmp(udev_device_get_sysname(dev), "ram", 3) == 0)
            continue;

        printf("==============================\n");
        printf("Device Node Path : %s\n", udev_device_get_devnode(dev));
        printf("==============================\n");
        printf("SERIAL  : %s\n", udev_device_get_property_value(dev, "ID_SERIAL_SHORT"));
        printf("MODEL   : %s\n", udev_device_get_property_value(dev, "ID_MODEL"));
        printf("VENDOR  : %s\n", udev_device_get_property_value(dev, "ID_VENDOR"));
        printf("DEVTYPE : %s\n", udev_device_get_devtype(dev));

        tmp = udev_device_get_property_value(dev, "ID_BUS");
        if (tmp != NULL) {
            if (strncmp(tmp, "ata", 3) == 0) {
                tmp = udev_device_get_property_value(dev, "ID_ATA_SATA");
                if ((tmp != NULL) && strncmp(tmp, "1", 1) == 0)
                    printf("BUSTYPE : sata\n");
            }
            else if (strncmp(tmp, "usb", 3) == 0)
                printf("BUSTYPE : usb\n");

        }
        printf("\n");

        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);

    udev_unref(udev);

    return 0;
}
