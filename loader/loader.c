typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef char bool;

typedef struct {
	uint8_t active;
	uint32_t size;
} alloc_info;

struct gdt_entry {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed));

struct gdt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct IDTPtr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct Register {
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags, useresp, ss;
};

struct IDT {
    unsigned short base_lo;
    unsigned short sel;
    unsigned char always0;
    unsigned char flags;
    unsigned short base_hi;
} __attribute__((packed));

typedef struct {
	uint32_t base;
	uint16_t byte_count;
	uint16_t rsv0:15;
	uint16_t eot:1;
} PRD __attribute__((packed));

struct IDT idts[256];
struct IDTPtr idt_ptr;
PRD prds[64] __attribute__((aligned(4)));

uint32_t alloc_top;

#define NULL 0
#define true 1
#define false 0
#define IDE 1
#define AHCI 2

#define FIRST_DEVICE 0
#define SECOND_DEVICE 1

#define FIRST_CONTROLLER 1
#define SECOND_CONTROLLER 2

#define IDE_COMPAT_MODE 1
#define NATIVE_PCI_MODE 2
int primary_ide_mode = IDE_COMPAT_MODE;
int secondary_ide_mode = IDE_COMPAT_MODE;

void* memset(void* ptr, int value, int count) {
	char* data = (char*)ptr;
	for (int i=0; i<count; i++) {
		data[i] = value;
	}
	return ptr;
}

void* malloc(int size) {
	alloc_info* info = (alloc_info*)alloc_top;
	while (info->active) {
		info = (alloc_info*)((uint32_t)info+sizeof(alloc_info)+info->size);
	}
	info->active = true;
	info->size = size;
	return (void*)((uint32_t)info+sizeof(alloc_info));
}

void init_alloc() {
	alloc_info* info = (alloc_info*)alloc_top;
	info->active = true;
	info->size = 10;
	info = (alloc_info*)(alloc_top+sizeof(alloc_info)+info->size);
	info->active = false;
}

typedef struct {
	uint32_t bus;
	uint32_t device;
	uint32_t func;
	uint32_t bar0;
	uint32_t bar1;
	uint32_t bar2;
	uint32_t bar3;
	uint32_t bar4;
	uint32_t bar5;
	uint16_t progIF;
	uint8_t intLine;
	uint8_t intPin;
} PCIDevice;

PCIDevice device;
int total_device = 0;
int mode = IDE;

int text_y = 0;

int get_offset(int x, int y) {
	return 2*(y*80+x);
}

void dump(char* buffer, int count) {
	char* scr = (char*)0xB8000;
	int offset = get_offset(0, text_y);
	for (int i=0; i<count; i++) {
		scr[offset] = buffer[i];
		scr[offset+1] = 0x0F;
		offset += 2;
	}
	text_y = offset/2/80+(((offset/2%80)!=0)?1:0);
}

void print(char* text) {
	char* scr = (char*)0xB8000;
	int i = 0;
	int j = get_offset(0, text_y);
	while (text[i] != 0) {
		scr[j] = text[i];
		scr[j+1] = 0x0F;
		j += 2;
		i++;
	}
	text_y++;
}

int numlen(int number) {
	int total = 0;
	if (number == 0) {
		return 1;
	}
	while (number > 0) {
		number /= 10;
		total++;
	}
	return total;
}

int numlenhex(int number) {
	int total = 0;
	if (number == 0) {
		return 1;
	}
	while (number > 0) {
		number /= 16;
		total++;
	}
	return total;
}

int abs(int number) {
	if (number < 0) {
		return -number;
	}
	return number;
}

void print_number(int number) {
	number = abs(number);
	int len = numlen(number)-1;
	char* scr = (char*)0xB8000;
	int offset = get_offset(0, text_y);
	offset += len*2;
	if (number == 0) {
		scr[offset] = '0';
		scr[offset+1] = 0x0F;
		text_y++;
		return;
	}
	while (number > 0) {
		scr[offset] = number%10+'0';
		scr[offset+1] = 0x0F;
		offset -= 2;
		number /= 10;
	}
	text_y++;
}

char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void print_number_hex(int number) {
	number = abs(number);
	int len = numlenhex(number)-1;
	char* scr = (char*)0xB8000;
	int offset = get_offset(0, text_y);
	offset += len*2;
	if (number == 0) {
		scr[offset] = '0';
		scr[offset+1] = 0x0F;
		text_y++;
		return;
	}
	while (number > 0) {
		scr[offset] = hex[number%16];
		scr[offset+1] = 0x0F;
		offset -= 2;
		number /= 16;
	}
	text_y++;
}

void out32(int port, uint32_t value) {
	asm volatile("out dx, eax"::"a"(value),"d"(port));
}

uint32_t in32(int port) {
	uint32_t value;
	asm volatile("in eax, dx":"=a"(value):"d"(port));
	return value;
}

void out16(int port, uint16_t value) {
	asm volatile("out dx, ax"::"a"(value),"d"(port));
}

uint16_t in16(int port) {
	uint16_t value;
	asm volatile("in ax, dx":"=a"(value):"d"(port));
	return value;
}

void out8(int port, uint8_t value) {
	asm volatile("out dx, al"::"a"(value),"d"(port));
}

uint8_t in8(int port) {
	uint8_t value;
	asm volatile("in al, dx":"=a"(value):"d"(port));
	return value;
}

void mmio_write_8(int base, uint8_t value) {
	uint8_t* data = (uint8_t*)base;
	data[0] = value;
}

uint8_t mmio_read_8(int base) {
	uint8_t* data = (uint8_t*)base;
	return data[0];
}

void mmio_write_16(int base, uint16_t value) {
	uint16_t* data = (uint16_t*)base;
	data[0] = value;
}

uint16_t mmio_read_16(int base) {
	uint16_t* data = (uint16_t*)base;
	return data[0];
}

void mmio_write_32(int base, uint32_t value) {
	uint32_t* data = (uint32_t*)base;
	data[0] = value;
}

uint32_t mmio_read_32(int base) {
	uint32_t* data = (uint32_t*)base;
	return data[0];
}

uint32_t pci_read_int(int bus, int device, int func, int offset) {
	int address = (offset&0xFC)|(func<<8)|(device<<11)|(bus<<16)|(1<<31);
	out32(0xCF8, address);
	uint32_t value = in32(0xCFC);
	return value;
}

void pci_write_int(int bus, int device, int func, int offset, int value) {
	int address = (offset&0xFC)|(func<<8)|(device<<11)|(bus<<16)|(1<<31);
	out32(0xCF8, address);
	out32(0xCFC, value);
	return value;
}

uint16_t pci_read_word(int bus, int device, int func, int offset) {
	int address = (offset&0xFC)|(func<<8)|(device<<11)|(bus<<16)|(1<<31);
	out32(0xCF8, address);
	uint32_t value = (in32(0xCFC)>>((offset&2)*8))&0xFFFF;
	return (uint16_t)value;
}

void pci_write_word(int bus, int device, int func, int offset, uint16_t value) {
	int address = (offset&0xFC)|(func<<8)|(device<<11)|(bus<<16)|(1<<31);
	out32(0xCF8, address);
	out16(0xCFC, value);
	return value;
}

PCIDevice* find_device2(int classCode, int subclassCode) {
	for (int bus=0; bus<256; bus++) {
		for (int deviceNumber=0; deviceNumber<32; deviceNumber++) {
			for (int func=0; func<8; func++) {
				int vendorID = pci_read_word(bus, deviceNumber, func, 0);
				if (vendorID == 0xFFFF) {
					continue;
				}
				int class = pci_read_word(bus, deviceNumber, func, 10)>>8&0x0F;
				int subclass = pci_read_word(bus, deviceNumber, func, 10)&0x0F;
				if (class == classCode && subclass == subclassCode) {
					device.bus = bus;
					device.device = deviceNumber;
					device.func = func;
					device.bar0 = pci_read_int(bus, deviceNumber, func, 16);
					device.bar1 = pci_read_int(bus, deviceNumber, func, 20);
					device.bar2 = pci_read_int(bus, deviceNumber, func, 24);
					device.bar3 = pci_read_int(bus, deviceNumber, func, 28);
					device.bar4 = pci_read_int(bus, deviceNumber, func, 32);
					device.bar5 = pci_read_int(bus, deviceNumber, func, 36);
					device.progIF = pci_read_word(bus, deviceNumber, func, 8)>>8&0xFF;
					device.intLine = pci_read_word(bus, deviceNumber, func, 60)&0xFF;
					device.intPin = pci_read_word(bus, deviceNumber, func, 60)>>8&0xFF;
					return &device;
				}
			}
		}
	}
	return NULL;
}

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
extern void syscall();
extern void flush_idt();
extern void irq_handler(struct Register _register_);

void irq_remap(void) {
    out8(0x20, 0x11);
    out8(0xA0, 0x11);
    out8(0x21, 0x20);
    out8(0xA1, 0x28);
    out8(0x21, 0x04);
    out8(0xA1, 0x02);
    out8(0x21, 0x01);
    out8(0xA1, 0x01);
    out8(0x21, 0x0);
    out8(0xA1, 0x0);
}

extern void fault_handler(struct Register _register_);

void fault_handler(struct Register _register_) {
	print("Fault caught");
	while (1);
}

void irq_handler(struct Register _register_) {
	if (_register_.int_no == 14 || _register_.int_no == 15) {
		print("Receive IRQ with number:");
		print_number(_register_.int_no);
		while (1);
	}
}

void syscall_handler(struct Register _register_) {
	print("IRQ caught");
	while (1);
}

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags) {
    idts[num].base_lo = (base & 0xFFFF);
    idts[num].base_hi = (base >> 16) & 0xFFFF;
    idts[num].sel = sel;
    idts[num].always0 = 0;
    idts[num].flags = flags;
}

void init_idt() {
	irq_remap();
	memset(&idts, 0, sizeof(struct IDT)*256);
    idt_set_gate(0, (unsigned)isr0, 0x08, 0x8E);
    idt_set_gate(1, (unsigned)isr1, 0x08, 0x8E);
    idt_set_gate(2, (unsigned)isr2, 0x08, 0x8E);
    idt_set_gate(3, (unsigned)isr3, 0x08, 0x8E);
    idt_set_gate(4, (unsigned)isr4, 0x08, 0x8E);
    idt_set_gate(5, (unsigned)isr5, 0x08, 0x8E);
    idt_set_gate(6, (unsigned)isr6, 0x08, 0x8E);
    idt_set_gate(7, (unsigned)isr7, 0x08, 0x8E);
    idt_set_gate(8, (unsigned)isr8, 0x08, 0x8E);
    idt_set_gate(9, (unsigned)isr9, 0x08, 0x8E);
    idt_set_gate(10, (unsigned)isr10, 0x08, 0x8E);
    idt_set_gate(11, (unsigned)isr11, 0x08, 0x8E);
    idt_set_gate(12, (unsigned)isr12, 0x08, 0x8E);
    idt_set_gate(13, (unsigned)isr13, 0x08, 0x8E);
    idt_set_gate(14, (unsigned)isr14, 0x08, 0x8E);
    idt_set_gate(15, (unsigned)isr15, 0x08, 0x8E);
    idt_set_gate(16, (unsigned)isr16, 0x08, 0x8E);
    idt_set_gate(17, (unsigned)isr17, 0x08, 0x8E);
    idt_set_gate(18, (unsigned)isr18, 0x08, 0x8E);
    idt_set_gate(19, (unsigned)isr19, 0x08, 0x8E);
    idt_set_gate(20, (unsigned)isr20, 0x08, 0x8E);
    idt_set_gate(21, (unsigned)isr21, 0x08, 0x8E);
    idt_set_gate(22, (unsigned)isr22, 0x08, 0x8E);
    idt_set_gate(23, (unsigned)isr23, 0x08, 0x8E);
    idt_set_gate(24, (unsigned)isr24, 0x08, 0x8E);
    idt_set_gate(25, (unsigned)isr25, 0x08, 0x8E);
    idt_set_gate(26, (unsigned)isr26, 0x08, 0x8E);
    idt_set_gate(27, (unsigned)isr27, 0x08, 0x8E);
    idt_set_gate(28, (unsigned)isr28, 0x08, 0x8E);
    idt_set_gate(29, (unsigned)isr29, 0x08, 0x8E);
    idt_set_gate(30, (unsigned)isr30, 0x08, 0x8E);
    idt_set_gate(31, (unsigned)isr31, 0x08, 0x8E);
    idt_set_gate(32, (unsigned)irq0, 0x08, 0x8E);
    idt_set_gate(33, (unsigned)irq1, 0x08, 0x8E);
    idt_set_gate(34, (unsigned)irq2, 0x08, 0x8E);
    idt_set_gate(35, (unsigned)irq3, 0x08, 0x8E);
    idt_set_gate(36, (unsigned)irq4, 0x08, 0x8E);
    idt_set_gate(37, (unsigned)irq5, 0x08, 0x8E);
    idt_set_gate(38, (unsigned)irq6, 0x08, 0x8E);
    idt_set_gate(39, (unsigned)irq7, 0x08, 0x8E);
    idt_set_gate(40, (unsigned)irq8, 0x08, 0x8E);
    idt_set_gate(41, (unsigned)irq9, 0x08, 0x8E);
    idt_set_gate(42, (unsigned)irq10, 0x08, 0x8E);
    idt_set_gate(43, (unsigned)irq11, 0x08, 0x8E);
    idt_set_gate(44, (unsigned)irq12, 0x08, 0x8E);
    idt_set_gate(45, (unsigned)irq13, 0x08, 0x8E);
    idt_set_gate(46, (unsigned)irq14, 0x08, 0x8E);
    idt_set_gate(47, (unsigned)irq15, 0x08, 0x8E);
    idt_set_gate(48, (unsigned)syscall, 0x08, 0x8E);
    idt_ptr.limit = sizeof(struct IDTPtr)*256-1;
    idt_ptr.base = (unsigned)&idts;
	flush_idt();
}

void find_kernel_ide() {
	int primary_ide_device = 0;
	int secondary_ide_device = 0;
	int ide_device = 0;
	int ide_controller = 0;
	// Check the mode suitable for primary controller
	if (((device.progIF>>1)&1)==0) {
		// This primary IDE mode is fixed
		if ((device.progIF&1)==0) {
			// This primary IDE has compatibility mode
			primary_ide_mode = IDE_COMPAT_MODE;
		} else if ((device.progIF&1)==1) {
			primary_ide_mode = NATIVE_PCI_MODE;
		}
	} else if (((device.progIF>>1)&1)==1) {
		// This primary IDE supports both mode, so choose native-pci one
		//device.progIF |= 1;
		device.progIF &= 0xFFFE;
		pci_write_word(device.bus, device.device, device.func, 8, device.progIF);
		primary_ide_mode = IDE_COMPAT_MODE;
	}
	// Check the mode suitable for secondary controller
	if (((device.progIF>>3)&1)==0) {
		// This primary IDE mode is fixed
		if (((device.progIF>>2)&1)==0) {
			// This primary IDE has compatibility mode
			secondary_ide_mode = IDE_COMPAT_MODE;
		} else if (((device.progIF>>2)&1)==1) {
			secondary_ide_mode = NATIVE_PCI_MODE;
		}
	} else if (((device.progIF>>3)&1)==1) {
		// This primary IDE supports both mode, so choose native-pci one
		//device.progIF |= (1<<2);
		device.progIF &= 0xFFFB;
		pci_write_word(device.bus, device.device, device.func, 8, device.progIF);
		secondary_ide_mode = IDE_COMPAT_MODE;
	}
	// Find for CD-drive
	int bar;
	if (primary_ide_mode == IDE_COMPAT_MODE) {
		print("The device is in compatibility mode");
		bar = 0x1F0;
		int interrupt = pci_read_word(device.bus, device.device, device.func, 60);
		interrupt &= 0xFF00;
		// Set primary ATA irq number to 14
		interrupt |= 14;
		pci_write_word(device.bus, device.device, device.func, 60, interrupt);
		device.intLine = interrupt&0xFF;
	} else if (primary_ide_mode == NATIVE_PCI_MODE) {
		print("The device is in native-pci mode");
		bar = device.bar0;
	}
	bool cd_drive_found = false;
	out8(bar+6, 0x0);
	out8(bar+1, 0x0);
	out8(bar+2, 0x0);
	out8(bar+3, 0x0);
	out8(bar+4, 0x0);
	out8(bar+5, 0x0);
	out8(bar+7, 0xEC);
	if ((in8(bar+7)&1)!=0) {
		// Error, the device implementing packet commands feature
		cd_drive_found = true;
		primary_ide_device = FIRST_DEVICE;
		ide_device = primary_ide_device;
		ide_controller = FIRST_CONTROLLER;
		goto read_kernel;
	} else {
		while (((in8(bar+7)>>7)&1)!=0) {}
	}
	if (!cd_drive_found) {
		out8(bar+6, 1<<4);
		out8(bar+1, 0);
		out8(bar+2, 0);
		out8(bar+3, 0);
		out8(bar+4, 0);
		out8(bar+5, 0);
		out8(bar+7, 0xEC);
		if ((in8(bar+7)&1)!=0) {
			// Error, the device implementing packet commands feature
			cd_drive_found = true;
			primary_ide_device = SECOND_DEVICE;
			ide_device = primary_ide_device;
			ide_controller = FIRST_CONTROLLER;
			goto read_kernel;
		} else {
			while (((in8(bar+7)>>7)&1)!=0) {}
		}
	}
	if (secondary_ide_mode == IDE_COMPAT_MODE) {
		print("The device is in compatibility mode");
		bar = 0x170;
		int interrupt = pci_read_word(device.bus, device.device, device.func, 60);
		interrupt &= 0xFF00;
		// Set primary ATA irq number to 15
		interrupt |= 15;
		pci_write_word(device.bus, device.device, device.func, 60, interrupt);
		device.intLine = interrupt&0xFF;
	} else if (secondary_ide_mode == NATIVE_PCI_MODE) {
		print("The device is in native-pci mode");
		bar = device.bar2;
	}
	if (!cd_drive_found) {
		out8(bar+6, 0x0);
		out8(bar+1, 0);
		out8(bar+2, 0);
		out8(bar+3, 0);
		out8(bar+4, 0);
		out8(bar+5, 0);
		out8(bar+7, 0xEC);
		if ((in8(bar+7)&1)!=0) {
			// Error, the device implementing packet commands feature
			cd_drive_found = true;
			secondary_ide_device = FIRST_DEVICE;
			ide_device = secondary_ide_device;
			ide_controller = SECOND_CONTROLLER;
			goto read_kernel;
		} else {
			while (((in8(bar+7)>>7)&1)!=0) {}
		}
	}
	if (!cd_drive_found) {
		out8(bar+6, 1<<4);
		out8(bar+1, 0);
		out8(bar+2, 0);
		out8(bar+3, 0);
		out8(bar+4, 0);
		out8(bar+5, 0);
		out8(bar+7, 0xEC);
		if ((in8(bar+7)&1)!=0) {
			// Error, the device implementing packet commands feature
			cd_drive_found = true;
			secondary_ide_device = SECOND_DEVICE;
			ide_device = secondary_ide_device;
			ide_controller = SECOND_CONTROLLER;
			goto read_kernel;
		} else {
			while (((in8(bar+7)>>7)&1)!=0) {}
		}
	}
	print("CD-Drive not found");
	return;
	read_kernel:;
	print("CD-Drive found");
	if (ide_controller == FIRST_CONTROLLER) {
		if (primary_ide_mode == IDE_COMPAT_MODE) {
			bar = 0x1F0;
		} else if (primary_ide_mode == NATIVE_PCI_MODE) {
			bar = device.bar0;
		}
	} else if (ide_controller == SECOND_CONTROLLER) {
		if (secondary_ide_mode == IDE_COMPAT_MODE) {
			bar = 0x170;
		} else if (secondary_ide_mode == NATIVE_PCI_MODE) {
			bar = device.bar2;
		}
	}
	char ident[512];
	out8(bar+6, ide_device<<4);
	out8(bar+7, 0xA1);
	if ((in8(bar+7)&1)!=0) {
		print("Error identifying IDE device");
		return;
	}
	while (((in8(bar+7)>>7)&1)!=0/* && (((in8(bar+7)>>3)&1)!=0)*/) {}
	int j = 0;
	for (int i=0; i<256; i++) {
		short value = in16(bar);
		ident[j] = value>>8&0xFF;
		ident[j+1] = value&0xFF;
		j += 2;
	}
	short* data = (short*)((int)ident+49*2);
	print_number_hex(data[0]);
	data = (short*)((int)ident+88*2);
	print_number_hex(data[0]);
	while (1);
	memset(prds, 0, sizeof(PRD)*64);
	prds[0].base = 0x1E00000;
	prds[0].byte_count = 1024;
	prds[0].eot = 1;
	// Enable Bus Master
	int command = pci_read_word(device.bus, device.device, device.func, 4);
	command |= (1<<2);
	pci_write_word(device.bus, device.device, device.func, 4, command);
	// bar = Bus Master Base Address
	bar = device.bar4;
	mmio_write_32(bar+4, (uint32_t)prds);
	// Set direction to read
	command = mmio_read_8(bar);
	command &= ~(1<<3);
	mmio_write_8(bar, command);
	// Clear interrupt and error bit
	command = mmio_read_8(bar+2);
	command &= ~(1<<1);
	command &= ~(1<<2);
	mmio_write_8(bar+2, command);
	// Set device
	out8(0x1F0+6, 0);
	// Set LBA
	out8(0x1F0+3, 0);
	out8(0x1F0+4, 0);
	out8(0x1F0+5, 0);
	// Set sector count
	out8(0x1F0+2, 1);
	//out8(0x1F0+7, 0xC8);
	// Start DMA transfer
	command = mmio_read_8(bar);
	command |= 1;
	mmio_write_8(bar, command);
	while (((in8(bar+7)>>7)&1)!=0);
	print("Data received");
	char buffer[512];
	j = 0;
	for (int i=0; i<256; i++) {
		short value = in16(0x1F0);
		buffer[j] = value>>8&0xFF;
		buffer[j+1] = value&0xFF;
		j += 2;
	}
	while (1);
}

void find_kernel_ahci() {
}

extern unsigned int loader_end;

struct gdt_entry gdt[3];
struct gdt_ptr gp;

void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access = access;
}

void init_gdt() {
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base = &gdt;
    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_flush();
}

void test_ide_irq(PCIDevice* device) {
	int irq = pci_read_word(device->bus, device->device, device->func, 60);
	irq &= 0xFF00;
	irq |= 15;
	pci_write_word(device->bus, device->device, device->func, 60, irq);
	char atapi_commands[12];
	memset(atapi_commands, 0, 12);
	atapi_commands[0] = 0x28;
	atapi_commands[5] = 16;
	atapi_commands[8] = 1;
	out8(0x176, 0);
	out8(0x171, 0);
	out8(0x174, 512&0xFF);
	out8(0x175, 512>>8&0xFF);
	out8(0x177, 0xA0);
	while (((in8(0x177)>>7)&1)!=0);
	int j = 0;
	for (int i=0; i<6; i++) {
		char value1 = atapi_commands[j+1];
		char value2 = atapi_commands[j];
		uint32_t value = ((int)value1<<8)|((int)value2);
		value &= 0xFFFF;
		out16(0x170, value);
		j += 2;
	}
	while (((in8(0x177)>>7)&1)!=0);
	int a = in8(0x174);
	int b = in8(0x175);
	int byteRead = (b<<8)|a;
	print_number(byteRead);
	char buffer[512];
	memset(buffer, 0, 512);
	j = 0;
	for (int i=0; i<byteRead/2; i++) {
		short value = in16(0x170);
		buffer[j] = value&0xFF;
		buffer[j+1] = value>>8&0xFF;
		j += 2;
	}
	dump(buffer, 512);
	print("Waiting for interrupt");
	while (1);
}

int main() {
	alloc_top = 0x1400000; //20 MB
	init_gdt();
	init_idt();
	asm volatile("sti");
	print("Setting IDT success");
	init_alloc();
	// Check for availability of IDE device
	PCIDevice* device = find_device2(0x1, 0x1);
	if (device == NULL) {
		device = find_device2(0x1, 0x6);
		// If not found, find AHCI
		if (device == NULL) {
			print("No IDE or AHCI attached");
			return 0;
		} else {
			mode = AHCI;
			//print("AHCI found");
		}
	} else {
		mode = IDE;
		//print("IDE found");
	}
	if (mode == IDE) {
		find_kernel_ide();
	} else if (mode == AHCI) {
		find_kernel_ahci();
	}
	return 0;
}
