# write file data to disk

# tool
DD = dd

# files

## FILE1 ## 
NAME_FILE1 = ./mario.nes
OFFS_FILE1 = 10000
CNTS_FILE1 = 100

## FILE2 ## 
NAME_FILE2 = ./FILE2.txt
OFFS_FILE2 = 10
CNTS_FILE2 = 2

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
all: img vhd vmdk

# .img file
img:
	$(DD) if=$(NAME_FILE1) of=$(HD_IMG) bs=512 seek=$(OFFS_FILE1) count=$(CNTS_FILE1) conv=notrunc

#	$(DD) if=$(NAME_FILE2) of=$(HD_IMG) bs=512 seek=$(OFFS_FILE2) count=$(CNTS_FILE2) conv=notrunc
	
# .vhd file
vhd:
	$(DD) if=$(NAME_FILE1) of=$(HD_VHD) bs=512 seek=$(OFFS_FILE1) count=$(CNTS_FILE1) conv=notrunc

#	$(DD) if=$(NAME_FILE2) of=$(HD_VHD) bs=512 seek=$(OFFS_FILE2) count=$(CNTS_FILE2) conv=notrunc
	
# .vmdk file
vmdk:
	$(DD) if=$(NAME_FILE1) of=$(HD_VMDK) bs=512 seek=$(OFFS_FILE1) count=$(CNTS_FILE1) conv=notrunc

#	$(DD) if=$(NAME_FILE2) of=$(HD_VMDK) bs=512 seek=$(OFFS_FILE2) count=$(CNTS_FILE2) conv=notrunc
	