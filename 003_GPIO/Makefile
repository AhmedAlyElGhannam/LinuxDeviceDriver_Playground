# for dynamic modules
obj-m := gpio_device.o gpio_driver.o
CURR_WORKING_DIR = $(shell pwd)
KDIR = /lib/modules/$(shell uname -r)/build

all:
	make -C ${KDIR} M=${CURR_WORKING_DIR} modules

clean:
	make -C ${KDIR} M=${CURR_WORKING_DIR} clean