// SPI IP Library (spi_ip.c)
// Abiria Placide

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: DE1-SoC Board

// Hardware configuration:
// HPS interface:
//   Mapped to offset of 0x8000 in light-weight MM interface aperature
//   IRQ80 is used as the interrupt interface to the HPS

//-----------------------------------------------------------------------------

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <asm/io.h> //iowrite, ioread, ioremap_nocache(platform specific)
#include "address_map.h"  // address map
#include "spi_regs.h"       // registers offsets

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

//-----------
//kernel info
//

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Abiria Placide");
MODULE_DESCRIPTION("SPI IP Driver");

//global vars
static unsigned int *base = NULL;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------


/*
uint32_t getDataReg()
{
    uint32_t value = *(base+OFS_DATA);
    return value;
}


uint32_t getStatusReg()
{
    uint32_t value = *(base+OFS_STATUS);
    return value;
}


//STATUS reg flags

bool getRxOverflow()
{
    uint8_t pin = 0;
    uint32_t value = *(base+OFS_STATUS);
    return (value >> pin) & 1;
}

bool getRxFull()
{
    uint8_t pin = 1;
    uint32_t value = *(base+OFS_STATUS);
    return (value >> pin) & 1;

}

bool getRxEmpty()
{
    uint8_t pin = 2;
    uint32_t value = *(base+OFS_STATUS);
    return (value >> pin) & 1;

}

bool getTxOverflow()
{
    uint8_t pin = 3;
    uint32_t value = *(base+OFS_STATUS);
    return (value >> pin) & 1;
}

bool getTxFull()
{
    uint8_t pin = 4;
    uint32_t value = *(base+OFS_STATUS);
    return (value >> pin) & 1;

}

bool getTxEmpty()
{
    uint8_t pin = 5 ;
    uint32_t value = *(base+OFS_STATUS);
    return (value >> pin) & 1;

}


void setDataReg(uint32_t value)
{
	//we dont want a read modify write, so we directly change the value
     *(base+OFS_DATA) = value;
}

void setControlReg(uint32_t value)
{
	*(base+OFS_CTRL) = value;
}

void modControlReg(uint32_t value)
{
	uint32_t temp = getControlReg();

	temp |= value;
	*(base+OFS_CTRL) = temp;
}

void clearControlReg(uint32_t value)
{
	*(base+OFS_CTRL) &= ~value;
}

void setStatusReg(uint32_t value)
{
	*(base+OFS_STATUS) = value;
}


//enable transmit/receiver/brd
void setCtrlEnableBit(bool bit)
{
    //this will turn on transmitter/receiver/brd
	if(bit)
		*(base+OFS_CTRL) |= (1 << 15);
	else
		*(base+OFS_CTRL) &= ~(1 << 15);
}

*/ 

uint32_t getControlReg(void)
{
    uint32_t value = ioread32(base+OFS_CTRL);
    return value;
}

void setControlReg(uint32_t value)
{
	iowrite32(value, base+OFS_CTRL);
}

void modControlReg(uint32_t value)
{
	uint32_t temp = getControlReg();
	temp |= value;
	iowrite32(temp, base+OFS_CTRL);
}

void clearControlReg(uint32_t value)
{
	iowrite32(value, base+OFS_CTRL);
}

void setBaudrateReg(uint32_t value)
{
     iowrite32(value, (base+OFS_BRD)); 
}

uint32_t getBaudrateReg(void)
{
    uint32_t value = ioread32(base+OFS_BRD);
    return value;
}

bool getRxEmpty(void)
{
    uint8_t pin = 2;
    uint32_t value = ioread32(base+OFS_STATUS);
    return (value >> pin) & 1;
}

void setDataReg(uint32_t value)
{
	//we dont want a read modify write, so we directly change the value
     iowrite32(value, base+OFS_DATA);
}
//kernal objects


//baud_rate
static uint32_t baudrate = 0;
module_param(baudrate, int, S_IRUGO);
MODULE_PARM_DESC(baudrate, "set baudrate");

static ssize_t storeBaudRate(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
	int result = kstrtouint(buffer, 0, &baudrate);
	if(result == 0)
		setBaudrateReg(baudrate);

	return count;
}

static ssize_t showBaudRate(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
	baudrate = getBaudrateReg();
	return sprintf(buffer, "%u\n", baudrate);
}

//baudrate attr
static struct kobj_attribute baudrateAttr = __ATTR(baudrate, 0664, showBaudRate, storeBaudRate);

//word size
static uint32_t word_size = 0;
module_param(word_size, int, S_IRUGO);
MODULE_PARM_DESC(word_size, "set word_size");

static ssize_t showWordSize(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
	word_size = getControlReg();
	word_size &= 0x1F; 
	return sprintf(buffer, "%u\n", word_size);
	
} 

static ssize_t storeWordSize(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
	int result = kstrtouint(buffer, 0, &word_size);
	unsigned int value = getControlReg();

	if(result == 0)
	{
		word_size  = word_size - 1;
		value &= ~(0x1F); //clear first 5 bits
		value |= word_size;
		iowrite32(value, (base+OFS_CTRL));
	}

	return count;
}

//word size attr
static struct kobj_attribute wordsizeAttr = __ATTR(word_size, 0664, showWordSize, storeWordSize);

//chip select
static uint32_t cs_select = 0;
module_param(cs_select, int, S_IRUGO);
MODULE_PARM_DESC(cs_select, "set cs_select");
static ssize_t showCS_Select(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
	cs_select = getControlReg();
	cs_select = (cs_select >> 13) & 0x3;
	return sprintf(buffer, "%u\n", cs_select);
} 

static ssize_t storeCS_Select(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
	int result = kstrtouint(buffer, 0, &cs_select);
	unsigned int value = getControlReg();

	if(result == 0)
	{
		value &= ~(0x6000); //clear  bits 14 and 13
		value |= cs_select << 13;
		iowrite32(value, (base+OFS_CTRL));
	}
	return count;
}

//Chip select Attr
static struct kobj_attribute cs_selectAttr = __ATTR(cs_select, 0664, showCS_Select, storeCS_Select);

//tx_data
static uint32_t tx_data = 0;
module_param(tx_data, int, S_IRUGO);
MODULE_PARM_DESC(tx_data, "set tx_data");

static ssize_t storeTxData(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
	int result = kstrtouint(buffer, 0, &tx_data);
	if(result == 0)
	{
		setDataReg(tx_data);
	}
	return count;
}

static ssize_t showTxData(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
	return sprintf(buffer, "%s\n", "NAN");
}
//TxData attribute
static struct kobj_attribute TxDataAttr = __ATTR(tx_data, 0664, showTxData, storeTxData);


//rx_data
static uint32_t rx_data = 0;
module_param(rx_data, int, S_IRUGO);
MODULE_PARM_DESC(rx_data, "set rx_data");

static ssize_t storeRxData(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
	int result = kstrtouint(buffer, 0, &rx_data);
	if(result == 0); //do nothing
	return count;
}

static ssize_t showRxData(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
	bool p = getRxEmpty();
	if(p)
	{
		return sprintf(buffer, "%s\n", "-1");
	}
	else
	{
		return sprintf(buffer, "%s\n", "Not EMPTY"); //[x]
	}
}
//RxData attribute
static struct kobj_attribute RxDataAttr = __ATTR(rx_data, 0664, showRxData, storeRxData);


/*

//cs_auto
static uint32_t cs_auto = 0;
module_param(cs_auto, int, S_IRUGO);
MODULE_PARM_DESC(cs_auto, "set cs_auto");

static ssize_t storeCS_Auto(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
	int t = strncmp(buffer, "true" count-1);
	int f = strncmp(buffer, "false" count-1);
	uint32_t value = ioread32(base+OFS_CTRL);

	if(t == 0)
	{
		//set cs auto
	}
	else if (f == 0)
	{	
		//clear cs auto
	}
}

static ssize_t showCS_Auto(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
	bool p = getRxEmpty();
	if(p)
	{
		return sprintf(buffer, "%s\n", "-1");
	}
	else
	{
		return sprintf(buffer, "%s\n", "Not EMPTY"); //[x]
	}
}
//RxData attribute
static struct kobj_attribute RxDataAttr = __ATTR(rx_data, 0664, showRxData, storeRxData);

//mode
static uint32_t mode = 0;
module_param(mode, int, S_IRUGO);
MODULE_PARM_DESC(mode, "set mode");
static ssize_t showMode(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
	mode = getControlReg();

	if(strncmp(buffer, "mode0", count-1) == 0)
	{
		mode = (mode >> 16) & 0x2;
		//show mode 0
		sprintf(buffer, "%u\n", mode);
	}

	else if(strncmp(buffer, "mode1", count-1) == 0)
	{
		mode = (mode >> 18) & 0x2;
		//show mode 1
		sprintf(buffer, "%u\n", mode);
	}

	else if(strncmp(buffer, "mode2", count-1) == 0)
	{
		mode = (mode >> 20) & 0x2;
		//show mode 2
		sprintf(buffer, "%u\n", mode);
	}

	else if(strncmp(buffer, "mode3", count-1) == 0)
	{
		mode = (mode >> 22) & 0x2;
		//show mode 3
		sprintf(buffer, "%u\n", mode);
	}
}

static ssize_t storeMode(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
	int result = kstrtouint(buffer, 0, &mode);
	unsigned int value = getControlReg();

	if(result == 0)
	{
		if(mode == 0)
		{
			value &= ~(3 << 16);
			iowrite32(value, (base+OFS_CTRL));
		}

		if(mode == 1)
		{
			value &= ~(3 << 18);
			value |= (1 << 18);
			iowrite32(value, (base+OFS_CTRL));
		}
		if(mode == 2)
		{
			value &= ~(3 << 20);
			value |= (2 << 20);
			iowrite32(value, (base+OFS_CTRL));
		}
		if(mode == 3)
		{
			value &= ~(3 << 22);
			value |= (3 << 22);
			iowrite32(value, (base+OFS_CTRL));
		}
	}

	return count;
}

static struct kobj_attribute modeAttr = __ATTR(mode, 0664, showMode, StoreMode);



static struct attribute *attrs0[] = {&modeAttr.attr, NULL};
static struct attribute_group group0 = 
{
	.name = "device";
	.attrs = attrs0;
}

*/


static struct kobject *kobj;
//init kernel moduel
static int __init initialize_module(void)
{
	int result;
	printk(KERN_INFO "SPI driver: starting\n");
	kobj = kobject_create_and_add("spi", kernel_kobj);
	if(!kobj)
	{
		printk(KERN_ALERT "SPI driver: failed to create and add kobj\n");
		return -ENOENT;
	}

//	result = sysfs_create_group(kobj, &group0);

	//add single sysfs files
	result = sysfs_create_file(kobj, &baudrateAttr.attr); //baudrate
	result = sysfs_create_file(kobj, &wordsizeAttr.attr); //wordSize
	result = sysfs_create_file(kobj, &cs_selectAttr.attr); //wordSize
	result = sysfs_create_file(kobj, &TxDataAttr.attr); //Txdata
	result = sysfs_create_file(kobj, &RxDataAttr.attr); //Rxdata
	
	//add grouped sysfs files

	base = (unsigned int*)ioremap_nocache(LW_BRIDGE_BASE + SPI_BASE_OFFSET, SPAN_IN_BYTES);

	if(base == NULL)
		return -ENODEV;
	printk(KERN_INFO "SPI driver: initialized\n");

	return 0;

}

static void __exit exit_module(void)
{
	kobject_put(kobj);
	sysfs_remove_file(kernel_kobj, &baudrateAttr.attr);
	sysfs_remove_file(kernel_kobj, &wordsizeAttr.attr);
	sysfs_remove_file(kernel_kobj, &cs_selectAttr.attr);
	sysfs_remove_file(kernel_kobj, &TxDataAttr.attr);
	sysfs_remove_file(kernel_kobj, &RxDataAttr.attr);
	printk(KERN_INFO "SPI driver: exiting");
}

module_init(initialize_module);
module_exit(exit_module);