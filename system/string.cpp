#include <system.h>
#include <system/memory.h>

static char hex_value[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

int numlen(int number) {
	if (number == 0) return 1;
	int len = 0;
	while (number > 0) {
		len++;
		number /= 10;
	}
	return len;
}

int numlenhex(int number) {
	if (number == 0) return 1;
	int len = 0;
	while (number > 0) {
		len++;
		number /= 16;
	}
	return len;
}

int numlenhexunsigned(unsigned int number) {
	if (number == 0) return 1;
	int len = 0;
	while (number > 0) {
		len++;
		number /= 16;
	}
	return len;
}

char uchartochar(unsigned char ch) {
	int s = (ch>=0 ? (int)ch : 256+(int)ch);
	return (char)s;
}

unsigned int uchartouint(unsigned char ch) {
	return ((unsigned int)ch)&0xFF;
}

short charstoshort(char b1, char b2) {
    int sb1 = (b1>=0 ? (int)b1 : 256+(int)b1);
    int sb2 = (b2>=0 ? (int)b2 : 256+(int)b2);
	return (short)(sb1*0x100 + sb2);
}

short charstoushort(char b1, char b2) {
    int sb1 = (b1>=0 ? (int)b1 : 256+(int)b1);
    int sb2 = (b2>=0 ? (int)b2 : 256+(int)b2);
    short result = (short)( sb1*0x100 + sb2 );
    return result > 0 ? result : (short)(result & 0x7FFF);
}

unsigned int ucharstouint(unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4) {
    unsigned int sb1 = uchartouint(b1);
    unsigned int sb2 = uchartouint(b2);
    unsigned int sb3 = uchartouint(b3);
    unsigned int sb4 = uchartouint(b4);
    unsigned int result = (sb1*0x1000000 + sb2*0x10000 + sb3*0x100 + sb4);
    return result;
}

unsigned int ucharstoushort(unsigned char b1, unsigned char b2) {
    unsigned int sb1 = uchartouint(b1);
    unsigned int sb2 = uchartouint(b2);
    unsigned int result = (sb1*0x100 + sb2);
    return result;
}

int charstouint(char b1, char b2, char b3, char b4) {
    int sb1 = (b1>=0 ? (int)b1 : 256+(int)b1);
    int sb2 = (b2>=0 ? (int)b2 : 256+(int)b2);
    int sb3 = (b3>=0 ? (int)b3 : 256+(int)b3);
    int sb4 = (b4>=0 ? (int)b4 : 256+(int)b4);
    int result = (sb1*0x1000000 + sb2*0x10000 + sb3*0x100 + sb4);
    return result > 0 ? result : (result&0x7FFFFFFF);
}

int charstoint(char b1, char b2, char b3, char b4) {
    int sb1 = (b1>=0 ? (int)b1 : 256+(int)b1);
    int sb2 = (b2>=0 ? (int)b2 : 256+(int)b2);
    int sb3 = (b3>=0 ? (int)b3 : 256+(int)b3);
    int sb4 = (b4>=0 ? (int)b4 : 256+(int)b4);
    return (sb1*0x1000000 + sb2*0x10000 + sb3*0x100 + sb4);
}

char* inttostr(int number) {
	int len = numlen(number);
	char* strNumber = (char*)malloc(len+2);
	strNumber[len] = 0;
	int i = len-1;
	while (number > 0) {
		strNumber[i] = number%10+'0';
		i--;
		number /= 10;
	}
	return strNumber;
}

char* inttohexstr(int number) {
	int len = numlenhex(number);
	char* strNumber = (char*)malloc(len+2);
	strNumber[len] = 0;
	int i = len-1;
	while (number > 0) {
		strNumber[i] = hex_value[number%16];
		i--;
		number /= 16;
	}
	return strNumber;
}

char* concat(char* text1, char* text2) {
	int len1 = strlen(text1);
	int len2 = strlen(text2);
	int len = len1+len2+1;
	char* newString = (char*)malloc(len);
	int i = 0;
	int j = 0;
	while (text1[j] != 0) {
		newString[i] = text1[j];
		i++;
		j++;
	}
	j = 0;
	while (text2[j] != 0) {
		newString[i] = text2[j];
		i++;
		j++;
	}
	newString[len-1] = 0;
	return newString;
}

int strlen(char* text) {
	int total = 0;
	while (text[total] != 0) {
		total++;
	}
	return total;
}

bool strcmp(char* text1, char* text2, int len) {
	int i = 0;
	while (i < len) {
		if (text1[i] != text2[i]) {
			return false;
		}
		i++;
	}
	return true;
}

bool strcmpf(char* text1, char* text2, int len, int from1, int from2) {
	int i = from1;
	int j = from2;
	while (i < len+from1) {
		if (text1[i] != text2[j]) {
			return false;
		}
		i++;
		j++;
	}
	return true;
}

int search_string(char* data, char* toSearch, int from, int max) {
	int len = strlen(toSearch);
	int i = from;
	while ((max==0)?true:i < max) {
		if (strcmpf(data, toSearch, len, i, 0)) {
			return i;
		}
		i++;
	}
	return -1;
}

int get_index_of_string(char* string, char* toSearch, int from, int max) {
	return search_string(string, toSearch, from, max);
}

int get_char_index_from_string(char* string, char _toSearch, int from, int max) {
	char toSearch[2];
	toSearch[0] = _toSearch;
	toSearch[1] = 0;
	return search_string(string, toSearch, from, max);
}

int get_last_char_index_from_string(char* string, char _toSearch) {
	int len = strlen(string);
	int i = len-1;
	while (i > 0) {
		if (string[i] == _toSearch) {
			return i;
		}
		i--;
	}
	return -1;
}

char* strcombine(char* text1, char* text2) {
	int len1 = strlen(text1);
	int len2 = strlen(text2);
	int len = len1+len2;
	char* newText = (char*)malloc(len+1);
	memcpy(newText, text1, len1);
	memcpy((char*)((int)newText+len1), text2, len2);
	newText[len] = 0;
	return newText;
}

char* strclone(char* text) {
	int len = strlen(text);
	char* cloneText = (char*)malloc(len+1);
	memcpy(cloneText, text, len);
	cloneText[len] = 0;
	return cloneText;
}

int get_total_char_from_string(char* text, char ch) {
	int i = 0;
	int total = 0;
	while (text[i] != 0) {
		if (text[i] == ch) {
			total++;
		}
		i++;
	}
	return total;
}
