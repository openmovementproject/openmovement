// Written to allow legacy/fast font support on new HW
// KL 19-05-2011
#ifndef BASIC_FONT_H
#define BASIC_FONT_H

void Display_gotoxy(unsigned char x_col, unsigned char y_row);			// goto xy coordinates

void Display_putc_basic(unsigned char c,unsigned char size);			// Put character on screen
/*
Valid sizes are 1,2,4,8
*/
void Display_clear_line(unsigned char line);							// Erase a line/page 0-7

void DisplayPrintLine(char* string, unsigned char line, unsigned char size);  	// set line as 0-7 for line start or as follows
#define NEXT_LINE		0x80
#define PREVIOUS_LINE   0x81

void Display_print_xy(char*, unsigned char x, unsigned char y, unsigned char size);
#endif

