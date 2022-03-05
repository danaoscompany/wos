#include <system.h>
#include <system/memory.h>

bool isPlaying = true;
int currentSong = 0;

void test1() {
	// Drawing graphics on memory address 0xBEEF0201
	char* scr = (char*)(0xBEEF0201);
	for (int i=0; i<800*600; i++) {
		scr[i] = 0xFF;
	}
}

void drawWindows8Logo() {
	fill_bg(0x000000);
	draw_image((char*)&windows10logo, 0, -125, CENTER_ON_SCREEN);
}

void drawingWindows8() {
	set_font((char*)&arial_font, ARIAL_FONT_SIZE);
	set_font_size(25);
	// Fading-In a transition, windows logo start displaying
	transition(FADE_IN, 100, drawWindows8Logo);
	// Halting windows 8 logo
	draw_image((char*)&windows10logo, 0, -125, CENTER_ON_SCREEN);
	flush();
	sleep(3000);
	schedule(10000, 1);
	while (true) {
		draw_animation((char*)&windows10loadingsprite, 75, 50, 0, 200, CENTER_ON_SCREEN, drawWindows8Logo);
		flush();
		if (isTimerOccured(1)) {
			break;
		}
		drawWindows8Logo();
		flush();
		sleep(300);
	}
	// Fading-Out a transition, windows logo start hiding
	transition(FADE_OUT, 10, drawWindows8Logo);
	loop();
}

void askingForShutdown_Activity() {
	while (true) {
		draw_image((char*)&wallpaper, 0, 0, 0);
		fill_rect(0, 570, 800, 30, 0x000000);
		draw_image((char*)&start_icon, 0, 571, 0);
		draw_image((char*)&file_manager_icon, 45, 573, 0);
		draw_image((char*)&shutdown_icon, 770, 573, 0);
		/*fill_rect(200, 190, 400, 210, 0xFFFFFF);
		fill_rect(190, 200, 420, 190, 0xFFFFFF);
		fill_circle(200, 200, 10, 0xFFFFFF);
		fill_circle(600, 200, 10, 0xFFFFFF);
		fill_circle(200, 388, 10, 0xFFFFFF);
		fill_circle(599, 389, 10, 0xFFFFFF);*/
		draw_gradient(200, 190, 400, 210, 0x373B44, 0x4286f4, 1);
		if (is_clicked(770, 573, 800, 600)) {
			return;
		}
		flush();
	}
}

void shut_down() {
}

extern "C" void loadPageDirectory(unsigned int*);
extern "C" void enablePaging();

void init_paging() {
	uint32_t page_directory[1024] __attribute__((aligned(4096)));
	for(int i=0; i<1024; i++) {
    	page_directory[i] = 0x00000002;
	}
	uint32_t first_page_table[1024][1024] __attribute__((aligned(4096)));
	uint32_t addr = 0;
	for (int j=0; j<1024; j++) {
		for(int i=0; i<1024; i++) {
    		first_page_table[j][i] = addr | 3;
    		addr += 4096;
		}
	}
	page_directory[0] = ((unsigned int)first_page_table) | 3;
	loadPageDirectory(page_directory);
	enablePaging();
}

int main(BootInfo* bootInfo) {
	dbg_init();
	init_memory(0x2000000, 0x8000000);
	clear_screen();
	init_graphics(bootInfo->vbe_mode_info);
	init_gdt();
	init_idt();
	init_isr();
	init_irq();
	init_syscalls();
	init_pit(1000);
	init_keyboard();
	init_mouse();
	init_paging();
	//init_notifications();
	asm volatile("sti");
	set_utc(7);
	set_time_server(CMOS);
	init_devices();
	enumerate_devices();
	init_ac97();
	//init_ide();
	//init_storages();
	//run_app((char*)&app1, 0, NULL);
	//printf("\nDone\n");
	//while (1);
	//File* wallpaper = open_file("C:/eiffel.bmp");
	set_font((char*)&arial_font, ARIAL_FONT_SIZE);
	draw_image((char*)&wallpaper, 0, 0, 0);
	//fill_rect(0, 0, 800, 600, 0xffffffff);
	init_windows();
	char* argv[3];
	argv[0] = (char*)&arial_font;
	argv[1] = 296712;
	argv[2] = "Halo dunia";
	//elf_run((uint8_t*)&program1, 3, argv);
	return 0;
	flush();
	set_screen(get_main_screen());
	
	draw_image((char*)&file_manager_icon, 0, 573, 0);
	draw_image((char*)&shutdown_icon, 725, 573, 0);
	if (is_clicked(770, 573, 800, 600)) {
		askingForShutdown_Activity();
	}
	/*Window* window = (Window*)malloc(sizeof(Window));
	window->x = 150;
	window->y = 50;
	window->width = 300;
	window->height = 200;
	window->backgroundColor = 0xff0000;
	window->borderWidth = 2;
	window->title = "Window 1";
	window->tag = 101;
	create_window(window);
	window = (Window*)malloc(sizeof(Window));
	window->x = 150;
	window->y = 150;
	window->width = 400;
	window->height = 400;
	window->backgroundColor = 0x00ff00;
	window->borderWidth = 2;
	window->title = "Window 2";
	window->tag = 102;
	create_window(window);
	window = (Window*)malloc(sizeof(Window));
	window->x = 100;
	window->y = 100;
	window->width = 200;
	window->height = 300;
	window->borderWidth = 2;
	window->backgroundColor = 0x0000ff;
	window->title = "Window 3";
	window->tag = 102;
	create_window(window);*/
	
	Window* window = (Window*)malloc(sizeof(Window));
	window->x = 50;
	window->y = 50;
	window->width = 600;
	window->height = 400;
	window->backgroundColor = 0xff0000;
	window->borderWidth = 2;
	window->title = "Window 1";
	window->tag = 101;
	window->bpp = get_bpp();
	window->Bpp = get_Bpp();
	create_window(window);
	window->buffer = (uint8_t*)malloc(window->width*window->height*window->Bpp);
	window_fill_rect(window, 0, 0, window->width, window->height, 0xff000000);
	flush_window(window);
	
	// Draw taskbar
	Window* taskbar = (Window*)malloc(sizeof(Window));
	taskbar->x = -10;
	taskbar->y = get_height()-30;
	taskbar->width = get_width()+20;
	taskbar->height = 30;
	taskbar->backgroundColor = 0xff0000;
	taskbar->borderWidth = 2;
	taskbar->title = NULL;
	taskbar->tag = 0;
	taskbar->bpp = get_bpp();
	taskbar->Bpp = get_Bpp();
	create_window(taskbar);
	taskbar->buffer = (uint8_t*)malloc(taskbar->width*taskbar->height*taskbar->Bpp);
	flush_window(taskbar);
	
	copy_screen(get_windows_buffer(), get_main_screen());
	while (1) {
		refresh_windows();
		refresh_mouse();
	}
	//printf("%d\n", file->size);
	//init_audio_driver();
	//drawingWindows8();
	return 0;
	
	set_font((char*)&arial_font, ARIAL_FONT_SIZE);
	set_font_size(50);
	while (1) {
		fill_bg(0xFFFFFF);
		TextField* tf = new TextField(50, 50, 300, 50, 0xFFFFFF, 0x000000, (char*)&arial_font, ARIAL_FONT_SIZE, 20, 0x000000);
		tf->setText("This is text");
		tf->apply();
		flush();
	}
	return 0;
}
