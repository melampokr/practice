#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <err.h>

#include <libudev.h>

#define DEVICE_NODE_CMD     "lsblk -Spnbo KNAME,SERIAL,VENDOR,MODEL,SIZE"
#define DEVICE_NODE_PRIFIX  "sd"
#define _PATH_SYS_BLOCK     "/sys/block"

struct blk_dev_info
{
    char *name;
    char *file_name;
    char *serial;
    char *vendor;
    char *model;
    char *bus_type;
};

static inline char __attribute__((warn_unused_result)) *xstrdup(const char *str)
{
    char *ret;

    if (!str)
        return NULL;

    ret = strdup(str);

    if (!ret)
        err(EXIT_FAILURE, "cannot duplicate string");

    return ret;
}

static struct dirent *xreaddir(DIR *dp)
{
    struct dirent *d;

    while ((d = readdir(dp))) {
        if (!strcmp(d->d_name, ".") ||
                !strcmp(d->d_name, ".."))
            continue;

        /* blacklist here? */
        break;
    }
    return d;
}

static char *get_device_path(const char *name)
{
    char path[PATH_MAX];

    snprintf(path, sizeof(path), "/dev/%s", name);
    return xstrdup(path);
}

static int is_sata(struct udev_device *dev) 
{
    int ret = 0;
    char *bus;

    bus = xstrdup(udev_device_get_property_value(dev, "ID_ATA_SATA"));

    if ((bus != NULL) && bus[0] == '1')
        ret = 1;

    free(bus);

    return ret;
}

static int is_usb(struct udev_device *dev)
{
    int ret = 0;
    char *bus;

    bus = xstrdup(udev_device_get_property_value(dev, "ID_BUS"));

    if ((bus != NULL) && (strncmp(bus, "usb", 3) == 0))
        ret = 1;

    free(bus);

    return ret;
}

static int is_parted_dev(struct udev_device *dev)
{
    int ret = 0;
    char *part_table_type;

    part_table_type = xstrdup(udev_device_get_property_value(dev, "ID_PART_TABLE_TYPE"));

    if ((is_sata(dev)) && (part_table_type != NULL)) {
        ret = 1;
    }

    free(part_table_type);

    return ret;
}

int main(int argc, char *argv[])
{
    struct blk_dev_info dev_info;
    struct udev *udev;
    struct udev_device *dev;
    DIR *dir;
    struct dirent *dir_entry;

    char *tmp;

    dir = opendir(_PATH_SYS_BLOCK);
    udev = udev_new();

    while ((dir_entry = xreaddir(dir)))
    {

        dev_info.name = xstrdup(dir_entry->d_name);
        dev = udev_device_new_from_subsystem_sysname(udev, "block", dev_info.name);

        if (!is_sata(dev) && !is_usb(dev))
                continue;

        if (is_parted_dev(dev))
            continue;

        dev_info.file_name = get_device_path(dev_info.name);
        dev_info.serial = xstrdup(udev_device_get_property_value(dev, "ID_SERIAL_SHORT"));
        dev_info.model = xstrdup(udev_device_get_property_value(dev, "ID_MODEL"));
        dev_info.vendor = xstrdup(udev_device_get_property_value(dev, "ID_VENDOR"));

        tmp = xstrdup(udev_device_get_property_value(dev, "ID_ATA_SATA"));
        if (tmp != NULL) {
            if (strncmp(tmp, "1", 1) == 0)
                dev_info.bus_type = xstrdup("sata");
        }
        free(tmp);

        tmp = xstrdup(udev_device_get_property_value(dev, "ID_BUS"));
        if (tmp != NULL) {
            if (strncmp(udev_device_get_property_value(dev, "ID_BUS"), "usb", 3) == 0)
                dev_info.bus_type = xstrdup("usb");
        }
        free(tmp);

        printf("==============================\n");
        printf("NAME    : %s\n", dev_info.name);
        printf("FILE    : %s\n", dev_info.file_name);
        printf("SERIAL  : %s\n", dev_info.serial);
        printf("MODEL   : %s\n", dev_info.model);
        printf("VENDOR  : %s\n", dev_info.vendor);
        printf("devtype : %s\n", udev_device_get_devtype(dev));
        printf("bustype : %s\n", (dev_info.bus_type != NULL) ? dev_info.bus_type : "");

        free(dev_info.name);
        free(dev_info.file_name);
        free(dev_info.serial);
        free(dev_info.model);
        free(dev_info.vendor);
        free(dev_info.bus_type);
    }

    return 0;
}
