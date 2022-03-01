#ifndef INFO_H
#define INFO_H

typedef struct {
	int flags;
	int mem_lower;
	int mem_upper;
	int boot_dev;
	int cmdline;
	int mods_count;
	int mods_addr;
	int syms0;
	int syms1;
	int syms2;
	int syms3;
	int mmap_len;
	int mmap_addr;
	int drives_len;
	int drives_addr;
	int config_table;
	int bootloader_name;
	int apm_table;
	int vbe_control_info;
	int vbe_mode_info;
	short vbe_mode;
	short vbe_interface_seg;
	short vbe_interface_offs;
	short vbe_interface_len;
} BootInfo;

#endif
