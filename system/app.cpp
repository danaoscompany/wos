#include <system.h>

void elf_run(char* data, int argc, char** argv) {
	ELFHeader* h = (ELFHeader*)data;
	int sh_count = (int)h->e_shnum;
	if (sh_count >= 1) {
		int sh_offs = (int)data+h->e_shoff;
		for (int i=0; i<sh_count; i++) {
			ELFSectionHeader* sh = (ELFSectionHeader*)sh_offs;
			if (sh->sh_type == 0x08) {
				memset(sh->sh_addr, 0, sh->sh_size);
			} else {
				memcpy(sh->sh_addr, (int)data+sh->sh_offset, sh->sh_size);
			}
			sh_offs += sizeof(ELFSectionHeader);
		}
		void (*go)();
		go = h->e_entry;
		go();
	}
}

void run_app(char* data, int argc, char** argv) {
	// Check format of the executable file
	if (data[0] == 0x7F && data[1] == 'E' && data[2] == 'L' && data[3] == 'F') {
		elf_run(data, argc, argv);
	}
}
