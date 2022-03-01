#ifndef STRING_H
#define STRING_H

int numlen(int number);
int numlenhex(int number);
int numlenhexunsigned(unsigned int number);
char uchartochar(unsigned char ch);
short charstoshort(char b1, char b2);
short charstoushort(char b1, char b2);
int charstouint(char b1, char b2, char b3, char b4);
int charstoint(char b1, char b2, char b3, char b4);
char* inttostr(int number);
char* concat(char* text1, char* text2);
int strlen(char* text);
char* inttohexstr(int number);
bool strcmp(char* text1, char* text2, int len);
bool strcmpf(char* text1, char* text2, int len, int from1, int from2);
int search_string(char* data, char* toSearch, int from, int max);
int get_index_of_string(char* string, char* toSearch, int from, int max);
int get_char_index_from_string(char* string, char _toSearch, int from, int max);
int get_last_char_index_from_string(char* string, char _toSearch);
char* strcombine(char* text1, char* text2);
unsigned int uchartouint(unsigned char ch);
unsigned int ucharstouint(unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4);
char* strclone(char* text);
int get_total_char_from_string(char* text, char ch);
unsigned int ucharstoushort(unsigned char b1, unsigned char b2);

#endif
