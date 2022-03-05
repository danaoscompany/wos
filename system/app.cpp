#include <system.h>

void elf_run(char* data, int argc, char** argv) {
	void (*go)(int argc, char** argv);
	/*log("Entry point: %x\n", (uint32_t)data);
	go = (uint32_t)data;
	int result = go();
	log("Result: %d\n", result);
	return;*/
	ELFHeader* h = (ELFHeader*)data;
	ELFProgramHeader* ph = (ELFProgramHeader*)(data+h->e_phoff);
	for(int i = 0; i < h->e_phnum; i++, ph++)
	{
		switch(ph->p_type)
		 {
		 	case 0:
		 		break;
		 	case 1:
		 		log("LOAD: offset 0x%x vaddr 0x%x paddr 0x%x filesz 0x%x memsz 0x%x\n",
		 				ph->p_offset, ph->p_vaddr, ph->p_paddr, ph->p_filesz, ph->p_memsz);
		 		memcpy(ph->p_vaddr, data + ph->p_offset, ph->p_filesz);
		 		break;
		 	default:
		 	 return 0;
		 }
	}
	log("Entry point: %x\n", h->e_entry);
	int (*func)(int argc, char** argv) = (int (*)(void))((uint32_t)h->e_entry);
	int x = func(argc, argv);
	log("Result: %d\n", x);
}

void run_app(char* data, int argc, char** argv) {
	// Check format of the executable file
	if (data[0] == 0x7F && data[1] == 'E' && data[2] == 'L' && data[3] == 'F') {
		elf_run(data, argc, argv);
	}
}
