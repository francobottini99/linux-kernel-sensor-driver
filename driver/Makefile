obj-m += sensors_drivers.o

sensors_drivers-y := src/drv.o src/dht11.o src/tfmini.o src/device.o src/device_queue.o src/cp210x.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

BUILD_DIR := $(PWD)/build
BIN_DIR := $(PWD)/bin

COMPILER_VERSION ?= gcc

default:
	$(MAKE) -C $(KDIR) M="$(PWD)" EXTRA_CFLAGS="-I$(PWD)/inc" CC="$(COMPILER_VERSION)" modules
	
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BIN_DIR)
	
	mv *.ko $(BIN_DIR)
	mv *.mod.* $(BUILD_DIR)
	mv *.mod $(BUILD_DIR)
	mv .*.cmd $(BUILD_DIR)
	mv *.o $(BUILD_DIR)
	mv *.order $(BUILD_DIR)
	mv *.symvers $(BUILD_DIR)
	
clean:
	$(MAKE) -C $(KDIR) M="$(PWD)" clean
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)
