
#include <stddef.h>
#include <xbook/pci.h>
#include <arch/io.h>
#include <arch/memory.h>
#include <xbook/debug.h>

// #define DEBUG_PCI

pci_device_t pci_device_table[PCI_MAX_DEVICE_NR];	/*device table*/

static void pci_device_bar_init(pci_device_bar_t *bar, unsigned int addr_reg_val, unsigned int len_reg_val)
{
	/*if addr is 0xffffffff, we set it to 0*/
    if (addr_reg_val == 0xffffffff) {
        addr_reg_val = 0;
    }
	/*we judge type by addr register bit 0, if 1, type is io, if 0, type is memory*/
    if (addr_reg_val & 1) {
        bar->type = PCI_BAR_TYPE_IO;
		bar->base_addr = addr_reg_val  & PCI_BASE_ADDR_IO_MASK;
        bar->length    = ~(len_reg_val & PCI_BASE_ADDR_IO_MASK) + 1;
    } else {
        bar->type = PCI_BAR_TYPE_MEM;
        bar->base_addr = addr_reg_val  & PCI_BASE_ADDR_MEM_MASK;
        bar->length    = ~(len_reg_val & PCI_BASE_ADDR_MEM_MASK) + 1;
    }
}

void pci_device_bar_dump(pci_device_bar_t *bar)
{
    printk(KERN_DEBUG "pci_device_bar_dump: type: %s\n", bar->type == PCI_BAR_TYPE_IO ? "io base address" : "mem base address");
    printk(KERN_DEBUG "pci_device_bar_dump: base address: %x\n", bar->base_addr);
    printk(KERN_DEBUG "pci_device_bar_dump: len: %x\n", bar->length);
}

static void pci_device_init(
    pci_device_t *device,
    unsigned char bus,
    unsigned char dev,
    unsigned char function,
    unsigned short vendor_id,
    unsigned short device_id,
    unsigned int class_code,
    unsigned char revision_id,
    unsigned char multi_function
) {
	/*set value to device*/
    device->bus = bus;
    device->dev = dev;
    device->function = function;

    device->vendor_id = vendor_id;
    device->device_id = device_id;
    device->multi_function = multi_function;
    device->class_code = class_code;
    device->revision_id = revision_id;
	int i;
    for (i = 0; i < PCI_MAX_BAR; i++) {
         device->bar[i].type = PCI_BAR_TYPE_INVALID;
    }
    device->irq_line = -1;
}

static unsigned int pci_read_config(unsigned int bus, unsigned int device, unsigned int function, unsigned int addr)
{
	unsigned int reg = 0x80000000;
	/*make config add register*/
	reg |= (bus & 0xFF) << 16;
    reg |= (device & 0x1F) << 11;
    reg |= (function & 0x7) << 8;
    reg |= (addr & 0xFF) & 0xFC;	/*bit 0 and 1 always 0*/
    io_mfence();
	/*pci_write_config to config addr*/
    out32(PCI_CONFIG_ADDR, reg);
    io_mfence();
    return in32(PCI_CONFIG_DATA);	/*return confige addr's data*/
}

static void pci_write_config(unsigned int bus, unsigned int device, unsigned int function, unsigned int addr, unsigned int val)
{
	unsigned int reg = 0x80000000;
	/*make config add register*/
	reg |= (bus & 0xFF) << 16;
    reg |= (device & 0x1F) << 11;
    reg |= (function & 0x7) << 8;
    reg |= (addr & 0xFF) & 0xFC;	/*bit 0 and 1 always 0*/
	io_mfence();
	/*pci_write_config to config addr*/
    out32(PCI_CONFIG_ADDR, reg);
    io_mfence();
	/*pci_write_config data to confige addr*/
    out32(PCI_CONFIG_DATA, val);
}


static pci_device_t *pci_alloc_device()
{
	int i;
	for (i = 0; i < PCI_MAX_DEVICE_NR; i++) {
		if (pci_device_table[i].flags == PCI_DEVICE_INVALID) {
			pci_device_table[i].flags = PCI_DEVICE_USING;
			return &pci_device_table[i];
		}
	}
	return NULL;
}
#if 0
static int pci_free_device(pci_device_t *device)
{
	int i;
	for (i = 0; i < PCI_MAX_DEVICE_NR; i++) {
		if (&pci_device_table[i] == device) {
			device->flags = PCI_DEVICE_INVALID;
			return 0;
		}
	}
	return -1;
}
#endif


void pci_device_dump(pci_device_t *device)
{
	//printk("status:      %d\n", device->flags);
    
    printk(KERN_DEBUG "pci_device_dump: vendor id:      0x%x\n", device->vendor_id);
    printk(KERN_DEBUG "pci_device_dump: device id:      0x%x\n", device->device_id);
	printk(KERN_DEBUG "pci_device_dump: class code:     0x%x\n", device->class_code);
    printk(KERN_DEBUG "pci_device_dump: revision id:    0x%x\n", device->revision_id);
    printk(KERN_DEBUG "pci_device_dump: multi function: %d\n", device->multi_function);
    printk(KERN_DEBUG "pci_device_dump: card bus CIS pointer: %x\n", device->card_bus_pointer);
    printk(KERN_DEBUG "pci_device_dump: subsystem vendor id: %x\n", device->subsystem_vendor_id);
    printk(KERN_DEBUG "pci_device_dump: subsystem device id: %x\n", device->subsystem_device_id);
    printk(KERN_DEBUG "pci_device_dump: expansion ROM base address: %x\n", device->expansion_rom_base_addr);
    printk(KERN_DEBUG "pci_device_dump: capability list pointer:  %x\n", device->capability_list);
    printk(KERN_DEBUG "pci_device_dump: irq line: %d\n", device->irq_line);
    printk(KERN_DEBUG "pci_device_dump: irq pin:  %d\n", device->irq_pin);
    printk(KERN_DEBUG "pci_device_dump: min Gnt: %d\n", device->min_gnt);
    printk(KERN_DEBUG "pci_device_dump: max Lat:  %d\n", device->max_lat);
    int i;
	for (i = 0; i < PCI_MAX_BAR; i++) {
		/*if not a invalid bar*/
        if (device->bar[i].type != PCI_BAR_TYPE_INVALID) {
            printk(KERN_DEBUG "pci_device_dump: bar %d:\n", i);
			pci_device_bar_dump(&device->bar[i]);
        }
    }
    printk("\n");
}

static void pci_scan_device(unsigned char bus, unsigned char device, unsigned char function)
{
	/*pci_read_config vendor id and device id*/
    unsigned int val = pci_read_config(bus, device, function, PCI_DEVICE_VENDER);
    unsigned int vendor_id = val & 0xffff;
    unsigned int device_id = val >> 16;
	/*if vendor id is 0xffff, it means that this bus , device not exist!*/
    if (vendor_id == 0xffff) {
        return;
    }
    
	/*alloc a pci device to store info*/
	pci_device_t *pci_dev = pci_alloc_device();
	if(pci_dev == NULL){
		return;
	}

	/*pci_read_config header type*/
    val = pci_read_config(bus, device, function, PCI_BIST_HEADER_TYPE_LATENCY_TIMER_CACHE_LINE);
    unsigned char header_type = ((val >> 16));
	/*pci_read_config command*/
    val = pci_read_config(bus, device, function, PCI_STATUS_COMMAND);

    pci_dev->command = val & 0xffff;
    pci_dev->status = (val >> 16) & 0xffff;
    
   // unsigned int command = val & 0xffff;
	/*pci_read_config class code and revision id*/
    val = pci_read_config(bus, device, function, PCI_CLASS_CODE_REVISION_ID);
    unsigned int classcode = val >> 8;
    unsigned char revision_id = val & 0xff;
	
	/*init pci device*/
    pci_device_init(pci_dev, bus, device, function, vendor_id, device_id, classcode, revision_id, (header_type & 0x80));
	
	/*init pci device bar*/
	int bar, reg;
    for (bar = 0; bar < PCI_MAX_BAR; bar++) {
        reg = PCI_BASS_ADDRESS0 + (bar*4);
		/*pci_read_config bass address[0~5] to get address value*/
        val = pci_read_config(bus, device, function, reg);
		/*set 0xffffffff to bass address[0~5], so that if we pci_read_config again, it's addr len*/
        pci_write_config(bus, device, function, reg, 0xffffffff);
       
	   /*pci_read_config bass address[0~5] to get addr len*/
		unsigned int len = pci_read_config(bus, device, function, reg);
        /*pci_write_config the io/mem address back to confige space*/
		pci_write_config(bus, device, function, reg, val);
		/*init pci device bar*/
        if (len != 0 && len != 0xffffffff) {
            pci_device_bar_init(&pci_dev->bar[bar], val, len);
        }
    }

    /* get card bus CIS pointer */
    val = pci_read_config(bus, device, function, PCI_CARD_BUS_POINTER);
    pci_dev->card_bus_pointer = val;

    /* get subsystem device id and vendor id */
    val = pci_read_config(bus, device, function, PCI_SUBSYSTEM_ID);
    pci_dev->subsystem_vendor_id = val & 0xffff;
    pci_dev->subsystem_device_id = (val >> 16) & 0xffff;
    
    /* get expansion ROM base address */
    val = pci_read_config(bus, device, function, PCI_EXPANSION_ROM_BASE_ADDR);
    pci_dev->expansion_rom_base_addr = val;
    
    /* get capability list */
    val = pci_read_config(bus, device, function, PCI_CAPABILITY_LIST);
    pci_dev->capability_list = val;
    
	/*get irq line and pin*/
    val = pci_read_config(bus, device, function, PCI_MAX_LNT_MIN_GNT_IRQ_PIN_IRQ_LINE);
    if ((val & 0xff) > 0 && (val & 0xff) < 32) {
        unsigned int irq = val & 0xff;
        pci_dev->irq_line = irq;
        pci_dev->irq_pin = (val >> 8)& 0xff;
    }
    pci_dev->min_gnt = (val >> 16) & 0xff;
    pci_dev->max_lat = (val >> 24) & 0xff;
    
#ifdef DEBUG_PCI
    printk(KERN_DEBUG "pci_scan_device: pci device at bus: %d, device: %d function: %d\n", 
        bus, device, function);
    pci_device_dump(pci_dev);
#endif
}

static void pci_scan_buses()
{
	unsigned int bus;
	unsigned char device, function;
	/*扫描每一条总线上的设备*/
    for (bus = 0; bus < PCI_MAX_BUS; bus++) {
        for (device = 0; device < PCI_MAX_DEV; device++) {
           for (function = 0; function < PCI_MAX_FUN; function++) {
				pci_scan_device(bus, device, function);
			}
        }
    }
}

pci_device_t* pci_get_device(unsigned int vendor_id, unsigned int device_id)
{
	int i;
	pci_device_t* device;
	
	for (i = 0; i < PCI_MAX_DEVICE_NR; i++) {
		device = &pci_device_table[i];
        
		if (device->flags == PCI_DEVICE_USING &&
            device->vendor_id == vendor_id && 
            device->device_id == device_id) {
			return device;
		}
	}
    return NULL;
}

pci_device_t* pci_get_device_by_class_code(unsigned int class, unsigned int sub_class)
{
	int i;
	pci_device_t* device;
	
    /* 构建类代码 */
    unsigned int class_code = ((class & 0xff) << 16) | ((sub_class & 0xff) << 8);

	for (i = 0; i < PCI_MAX_DEVICE_NR; i++) {
		device = &pci_device_table[i];
		if (device->flags == PCI_DEVICE_USING &&
            (device->class_code & 0xffff00) == class_code) {
			return device;
		}
	}
    return NULL;
}

/* Search for a pci device based on vendor and device ID */
pci_device_t *pci_locate_device(unsigned short vendor, unsigned short device)
{
	if(vendor == 0xFFFF || device == 0xFFFF)
		return NULL;

    pci_device_t* tmp;
	int i;
	for (i = 0; i < PCI_MAX_DEVICE_NR; i++) {
		tmp = &pci_device_table[i];
		if (tmp->flags == PCI_DEVICE_USING &&
            tmp->vendor_id == vendor && 
            tmp->device_id == device) {
			return tmp;
		}
	}
	return NULL;
}

pci_device_t *pci_locate_class(unsigned short class, unsigned short _subclass)
{
	if(class == 0xFFFF || _subclass == 0xFFFF)
		return NULL;
	pci_device_t *tmp;
	/* 构建类代码 */
    unsigned int class_code = ((class & 0xff) << 16) | ((_subclass & 0xff) << 8);
    int i;
	for (i = 0; i < PCI_MAX_DEVICE_NR; i++) {
		tmp = &pci_device_table[i];
		if (tmp->flags == PCI_DEVICE_USING &&
            (tmp->class_code & 0xffff00) == class_code) {
			return tmp;
		}
	}
	return NULL;
}

void pci_enable_bus_mastering(pci_device_t *device)
{
    unsigned int val = pci_read_config(device->bus, device->dev, device->function, PCI_STATUS_COMMAND);
#ifdef DEBUG_PCI
    printk(KERN_DEBUG "pci_enable_bus_mastering: before command: %x\n", val);    
#endif
	val |= 4;
    pci_write_config(device->bus, device->dev, device->function, PCI_STATUS_COMMAND, val);

    val = pci_read_config(device->bus, device->dev, device->function, PCI_STATUS_COMMAND);
#ifdef DEBUG_PCI
    printk(KERN_DEBUG "pci_enable_bus_mastering: after command: %x\n", val);    
#endif
}

/*read value from pci device config space register*/
unsigned int pci_device_read(pci_device_t *device, unsigned int reg)
{
    return pci_read_config(device->bus, device->dev, device->function, reg);
}

/*write value to pci device config space register*/
void pci_device_write(pci_device_t *device, unsigned int reg, unsigned int value)
{
    pci_write_config(device->bus, device->dev, device->function, reg, value);
}

unsigned int pci_device_get_io_addr(pci_device_t *device)
{
	int i;
    for (i = 0; i < PCI_MAX_BAR; i++) {
        if (device->bar[i].type == PCI_BAR_TYPE_IO) {
            return device->bar[i].base_addr;
        }
    }

    return 0;
}

unsigned int pci_device_get_mem_addr(pci_device_t *device)
{
	int i;
    for (i = 0; i < PCI_MAX_BAR; i++) {
        if (device->bar[i].type == PCI_BAR_TYPE_MEM) {
            return device->bar[i].base_addr;
        }
    }

    return 0;
}

unsigned int pci_device_get_mem_len(pci_device_t *device)
{
    int i;
    for(i=0; i<PCI_MAX_BAR; i++) {
        if(device->bar[i].type == PCI_BAR_TYPE_MEM) {
            return device->bar[i].length;
        }
    }
    return 0;
}

unsigned int pci_device_get_irq_line(pci_device_t *device)
{
    return device->irq_line;
}

static unsigned int pic_get_device_connected()
{
	int i;
	pci_device_t *device;
	for (i = 0; i < PCI_MAX_BAR; i++) {
		device = &pci_device_table[i];
        if (device->flags != PCI_DEVICE_USING) {
            break;
        }
    }
	return i;
}

void init_pci()
{
    /*init pci device table*/
	int i;
	for (i = 0; i < PCI_MAX_DEVICE_NR; i++) {
		pci_device_table[i].flags = PCI_DEVICE_INVALID;
	}

	/*scan all pci buses*/
	pci_scan_buses();

    printk(KERN_INFO "init_pci: pci type device found %d.\n", pic_get_device_connected());
}
