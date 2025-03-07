# for dynamic modules
obj-m += gpiotoggle.o

KDIR = /lib/modules/$(shell uname -r)/build
CURR_WORKING_DIR = $(shell pwd)

all:
	make -C ${KDIR} M=${CURR_WORKING_DIR} modules

clean:
	make -C ${KDIR} M=${CURR_WORKING_DIR} clean