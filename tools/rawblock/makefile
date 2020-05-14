# write file data to disk

# tool
DD = dd

# files

## test ## 
NAME_TEST = ./test.txt
OFFS_TEST = 0
CNTS_TEST = 1
SIZE_TEST = 1

## test2 ## 
NAME_TEST2 = ./test2.txt
OFFS_TEST2 = 10
CNTS_TEST2 = 2
SIZE_TEST = 1

#img dir
IMG_DIR = ../../develop/image/

DISK_IMG = c.img
DISK_VHD = c.vhd
DISK_VMDK = c-flat.vmdk

HD_IMG = $(IMG_DIR)$(DISK_IMG)
HD_VHD = $(IMG_DIR)$(DISK_VHD)
HD_VMDK = $(IMG_DIR)$(DISK_VMDK)

#First read here
.PHONY: all

# default work
all: img

# .img file
img:
	$(DD) if=$(NAME_TEST) of=$(HD_IMG) bs=512 seek=$(OFFS_TEST) count=$(CNTS_TEST) conv=notrunc
	$(DD) if=$(NAME_TEST2) of=$(HD_IMG) bs=512 seek=$(OFFS_TEST2) count=$(CNTS_TEST2) conv=notrunc
	
# .vhd file
vhd:
	$(DD) if=$(NAME_TEST) of=$(HD_VHD) bs=512 seek=$(OFFS_TEST) count=$(CNTS_TEST) conv=notrunc
	$(DD) if=$(NAME_TEST2) of=$(HD_VHD) bs=512 seek=$(OFFS_TEST2) count=$(CNTS_TEST2) conv=notrunc
	
# .vmdk file
vmdk:
	$(DD) if=$(NAME_TEST) of=$(HD_VMDK) bs=512 seek=$(OFFS_TEST) count=$(CNTS_TEST) conv=notrunc
	$(DD) if=$(NAME_TEST2) of=$(HD_VMDK) bs=512 seek=$(OFFS_TEST2) count=$(CNTS_TEST2) conv=notrunc
	