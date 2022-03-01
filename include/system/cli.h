#ifndef CLI_H
#define CLI_H

void printf(char* text, ...);
void resetf();
void clear_screen();
void dump(char* data, int count);
void print_hex_number(int number, int x, int y);
void print_text(char* text, int x, int y);

#endif
