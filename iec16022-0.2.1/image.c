/** 
 *
 * Image handling tools, (c) AJK 2001-2005
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include "image.h"

#define	INTERLACE
#define CLEAR
#define USEZLIB

#ifdef USEZLIB
#include <zlib.h>
#endif

unsigned char const bbc[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	//  
	0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x00,	// !
	0x6C, 0x6C, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00,	// "
	0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00,	// #
	0x0C, 0x3F, 0x68, 0x3E, 0x0B, 0x7E, 0x18, 0x00,	// $
	0x60, 0x66, 0x0C, 0x18, 0x30, 0x66, 0x06, 0x00,	// %
	0x38, 0x6C, 0x6C, 0x38, 0x6D, 0x66, 0x3B, 0x00,	// &
	0x0C, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00,	// '
	0x0C, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x00,	// (
	0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00,	// )
	0x00, 0x18, 0x7E, 0x3C, 0x7E, 0x18, 0x00, 0x00,	// *
	0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00,	// +
	0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30,	// ,
	0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00,	// -
	0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00,	// .
	0x00, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x00, 0x00,	// /
	0x18, 0x24, 0x66, 0x66, 0x66, 0x24, 0x18, 0x00,	// 0 (non crossed)
	0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00,	// 1
	0x3C, 0x66, 0x06, 0x0C, 0x18, 0x30, 0x7E, 0x00,	// 2
	0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00,	// 3
	0x0C, 0x1C, 0x3C, 0x6C, 0x7E, 0x0C, 0x0C, 0x00,	// 4
	0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00,	// 5
	0x1C, 0x30, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00,	// 6
	0x7E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00,	// 7
	0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00,	// 8
	0x3C, 0x66, 0x66, 0x3E, 0x06, 0x0C, 0x38, 0x00,	// 9
	0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00,	// :
	0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x30,	// ;
	0x0C, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0C, 0x00,	// <
	0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00,	// =
	0x30, 0x18, 0x0C, 0x06, 0x0C, 0x18, 0x30, 0x00,	// >
	0x3C, 0x66, 0x0C, 0x18, 0x18, 0x00, 0x18, 0x00,	// ?
	0x3C, 0x66, 0x6E, 0x6A, 0x6E, 0x60, 0x3C, 0x00,	// @
	0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00,	// A
	0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00,	// B
	0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00,	// C
	0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00,	// D
	0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00,	// E
	0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00,	// F
	0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3C, 0x00,	// G
	0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00,	// H
	0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00,	// I
	0x3E, 0x0C, 0x0C, 0x0C, 0x0C, 0x6C, 0x38, 0x00,	// J
	0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00,	// K
	0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00,	// L
	0x63, 0x77, 0x7F, 0x6B, 0x6B, 0x63, 0x63, 0x00,	// M
	0x66, 0x66, 0x76, 0x7E, 0x6E, 0x66, 0x66, 0x00,	// N
	0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00,	// O
	0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00,	// P
	0x3C, 0x66, 0x66, 0x66, 0x6A, 0x6C, 0x36, 0x00,	// Q
	0x7C, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0x66, 0x00,	// R
	0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00,	// S
	0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00,	// T
	0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00,	// U
	0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00,	// V
	0x63, 0x63, 0x6B, 0x6B, 0x7F, 0x77, 0x63, 0x00,	// W
	0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00,	// X
	0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00,	// Y
	0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00,	// Z
	0x7C, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7C, 0x00,	// [
	0x00, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x00, 0x00,	// 
	0x3E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x3E, 0x00,	// ]
	0x18, 0x3C, 0x66, 0x42, 0x00, 0x00, 0x00, 0x00,	// ^
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,	// _
	0x1C, 0x36, 0x30, 0x7C, 0x30, 0x30, 0x7E, 0x00,	// `
	0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x3E, 0x00,	// a
	0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x7C, 0x00,	// b
	0x00, 0x00, 0x3C, 0x66, 0x60, 0x66, 0x3C, 0x00,	// c
	0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x3E, 0x00,	// d
	0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00,	// e
	0x1C, 0x30, 0x30, 0x7C, 0x30, 0x30, 0x30, 0x00,	// f
	0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x3C,	// g
	0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00,	// h
	0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00,	// i
	0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x70,	// j
	0x60, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0x00,	// k
	0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00,	// l
	0x00, 0x00, 0x36, 0x7F, 0x6B, 0x6B, 0x63, 0x00,	// m
	0x00, 0x00, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00,	// n
	0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00,	// o
	0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60,	// p
	0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x07,	// q
	0x00, 0x00, 0x6C, 0x76, 0x60, 0x60, 0x60, 0x00,	// r
	0x00, 0x00, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x00,	// s
	0x30, 0x30, 0x7C, 0x30, 0x30, 0x30, 0x1C, 0x00,	// t
	0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3E, 0x00,	// u
	0x00, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00,	// v
	0x00, 0x00, 0x63, 0x6B, 0x6B, 0x7F, 0x36, 0x00,	// w
	0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x00,	// x
	0x00, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x3C,	// y
	0x00, 0x00, 0x7E, 0x0C, 0x18, 0x30, 0x7E, 0x00,	// z
	0x0C, 0x18, 0x18, 0x70, 0x18, 0x18, 0x0C, 0x00,	// {
	0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00,	// |
	0x30, 0x18, 0x18, 0x0E, 0x18, 0x18, 0x30, 0x00,	// }
	0x31, 0x6B, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00,	// ~
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	//
};

const char smallc[] = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-+&()/[];%";
unsigned char const small[] = {
	0x00, 0x00, 0x00,	//
	0x1F, 0x11, 0x1F,	//0
	0x11, 0x1F, 0x10,	//1
	0x1D, 0x15, 0x17,	//2
	0x11, 0x15, 0x1F,	//3
	0x07, 0x04, 0x1F,	//4
	0x17, 0x15, 0x1D,	//5
	0x1F, 0x15, 0x1D,	//6
	0x01, 0x01, 0x1F,	//7
	0x1F, 0x15, 0x1F,	//8
	0x17, 0x15, 0x1F,	//9
	0x1E, 0x05, 0x1E,	//A
	0x1F, 0x15, 0x0A,	//B
	0x0E, 0x11, 0x11,	//C
	0x1F, 0x11, 0x0E,	//D
	0x1F, 0x15, 0x11,	//E
	0x1F, 0x05, 0x01,	//F
	0x0E, 0x11, 0x19,	//G
	0x1F, 0x04, 0x1F,	//H
	0x11, 0x1F, 0x11,	//I
	0x11, 0x0F, 0x01,	//J
	0x1F, 0x04, 0x1B,	//K
	0x1F, 0x10, 0x10,	//L
	0x1F, 0x03, 0x1F,	//M
	0x1F, 0x01, 0x1F,	//N
	0x0E, 0x11, 0x0E,	//O
	0x1F, 0x05, 0x02,	//P
	0x0E, 0x19, 0x1E,	//Q
	0x1F, 0x05, 0x1A,	//R
	0x12, 0x15, 0x09,	//S
	0x01, 0x1F, 0x01,	//T
	0x1F, 0x10, 0x1F,	//U
	0x0F, 0x10, 0x0F,	//V
	0x1F, 0x18, 0x1F,	//W
	0x1B, 0x04, 0x1B,	//X
	0x03, 0x1C, 0x03,	//Y
	0x19, 0x15, 0x13,	//Z
	0x04, 0x04, 0x04,	//-
	0x04, 0x0E, 0x04,	//+
	0x04, 0x0E, 0x04,	//& (+)
	0x00, 0x0E, 0x11,	//(
	0x11, 0x0E, 0x00,	//)
	0x08, 0x04, 0x02,	///
	0x00, 0x1F, 0x11,	//[
	0x11, 0x1F, 0x00,	//]
	0x10, 0x0A, 0x00,	//;
	0x09, 0x04, 0x12,	//%
};

Image *ImageNew(int w, int h, int c)
{				// create a new blank image
	Image *i;
	if (!w || !h)
		return 0;
	i = malloc(sizeof(*i));
	if (!i)
		return 0;
	memset(i, 0, sizeof(*i));
	i->W = w;
	i->L = w + 1;
	i->H = h;
	i->C = c;
	i->Image = malloc((w + 1) * h);
	if (!i->Image) {
		free(i);
		return 0;
	}
	memset(i->Image, 0, (w + 1) * h);
	if (c) {
		i->Colour = malloc(sizeof(Colour) * c);
		if (!i->Colour) {
			free(i->Image);
			free(i);
			return 0;
		}
		memset(i->Colour, 0, sizeof(Colour) * c);
	}
	return i;
}

void ImageFree(Image * i)
{				// free an image
	if (i) {
		if (i->Image)
			free(i->Image);
		if (i->Colour)
			free(i->Colour);
		free(i);
	}
}

#define	MAXLZW	4096
typedef short LZW[256];
typedef LZW LZWTree[MAXLZW];
typedef struct strPrivate {
	int cols;		// number of colours, power of 2
	unsigned char colbits;	// number of bits for colours
	int fh;			// file handle
	int lzwnext;		// next code
	int lzwlast;		// last code in current bit size
	int lzwbits;		// current bit size
	LZWTree lzw;		// encode tree
	unsigned char block[256];	// block so far, with count at start
	int blockv;		// pending value
	int blockb;		// bits used in pending value
	short lzwcode;		// which code we are on now
} Private;

static LZWFlush(Private * p)
{				// flush this block
	write(p->fh, p->block, *p->block + 1);
	*p->block = 0;
}

static LZWOut(Private * p, short v)
{				// output a value
	p->blockv |= (v << p->blockb);
	p->blockb += p->lzwbits;
	while (p->blockb >= 8) {
		p->block[++*p->block] = p->blockv;	// last partial byte
		p->blockv >>= 8;
		p->blockb -= 8;
		if (*p->block == 255)
			LZWFlush(p);
	}
}

static LZWClear(Private * p)
{
	int c;
	p->lzwbits = p->colbits + 1;
	p->lzwnext = p->cols + 2;
	p->lzwlast = (1 << p->lzwbits) - 1;
	p->lzwcode = p->cols;	// starting point
	for (c = 0; c < p->cols; c++) {
		p->lzw[p->cols][c] = c;	// links to literal entries
		// links from literals, dead ends initially
		memset(&p->lzw[c], -1, p->cols * 2);
	}
}

static ImageStart(Private * p)
{
	unsigned char b = p->colbits;
	write(p->fh, &b, 1);
	*p->block = 0;
	p->blockb = 0;
	p->blockv = 0;
	LZWClear(p);
	LZWOut(p, p->cols);	// clear code
}

static ImageEnd(Private * p)
{
	LZWOut(p, p->lzwcode);	// last prefix
	LZWOut(p, p->cols + 1);	// end code
	if (p->blockb)
		p->block[++*p->block] = p->blockv;	// last partial byte
	LZWFlush(p);
}

static ImageOut(Private * p, unsigned char c)
{
	short next = p->lzw[p->lzwcode][c];
	if (next == -1) {	// dead end
		LZWOut(p, p->lzwcode);	// prefix
#ifdef CLEAR
		if (p->lzwnext + 1 == MAXLZW) {
			LZWOut(p, p->cols);	// clear code
			LZWClear(p);
		} else
#endif
		if (p->lzwnext < MAXLZW) {
			memset(p->lzw[p->lzwnext], -1, p->cols * 2);	// init dead ends
			p->lzw[p->lzwcode][c] = p->lzwnext;
			if (p->lzwnext > p->lzwlast) {	// bigger code
				p->lzwbits++;
				p->lzwlast = (1 << p->lzwbits) - 1;
			}
			p->lzwnext++;
		}
		p->lzwcode = c;
	} else
		p->lzwcode = next;	// not a dead end
}

// write GIF image
void ImageWriteGif(Image * i, int fh, int back, int trans, char *comment)
{
	struct strPrivate p;
	p.fh = fh;
	// count colours, min 4
	for (p.colbits = 2, p.cols = 4; p.cols < i->C;
	     p.cols *= 2, p.colbits++) ;
	{			// headers
		char buf[1500];
		int n = 0;
		strcpy(buf, "GIF87a");
#ifndef INTERLACE
		if (comment || trans >= 0)
#endif
			buf[4] = '9';	// needs gif89 format
		n = 6;
		buf[n++] = (i->W & 255);
		buf[n++] = (i->W >> 8);
		buf[n++] = (i->H & 255);
		buf[n++] = (i->H >> 8);
		buf[n++] = (i->Colour ? 0x80 : 0) + 0x70 + (p.colbits - 1);
		buf[n++] = back;	// background
		buf[n++] = 0;	// aspect
		if (i->Colour) {
			int c;
			for (c = 0; c < p.cols; c++) {
				if (c < i->C) {
					buf[n++] = (i->Colour[c] >> 16 & 255);
					buf[n++] = (i->Colour[c] >> 8 & 255);
					buf[n++] = (i->Colour[c] & 255);
				} else {	// extra, unused, colour
					buf[n++] = 0;
					buf[n++] = 0;
					buf[n++] = 0;
				}
			}
		}
		// comment
		if (comment && strlen(comment) < 256) {	// comment
			buf[n++] = 0x21;	//extension
			buf[n++] = 0xFE;	//comment
			buf[n++] = strlen(comment);
			strcpy(buf + n, comment);
			n += buf[n - 1];
			buf[n++] = 0;	// end of block
		}
		if (trans >= 0) {	// transparrent
			buf[n++] = 0x21;	// extension
			buf[n++] = 0xF9;	// graphic control
			buf[n++] = 4;	// len
			buf[n++] = 1;	// transparrent
			buf[n++] = 0;	// delay
			buf[n++] = 0;
			buf[n++] = trans;
			buf[n++] = 0;	// terminator
		}
		// image
		buf[n++] = 0x2C;
		buf[n++] = 0;	// offset X
		buf[n++] = 0;
		buf[n++] = 0;	// offset Y
		buf[n++] = 0;
		buf[n++] = (i->W & 255);
		buf[n++] = (i->W >> 8);
		buf[n++] = (i->H & 255);
		buf[n++] = (i->H >> 8);
#ifdef INTERLACE
		buf[n++] = 0x40;	// interlaced, no local colour table
#else
		buf[n++] = 0x00;	// non interlaced, no local colour table
#endif
		write(fh, buf, n);
	}
	// image data
	{
		unsigned char *b;
		int x, y;
		ImageStart(&p);
#ifdef INTERLACE
		for (y = 0; y < i->H; y += 8)
			for (b = &ImagePixel(i, 0, y), x = 0; x < i->W; x++)
				ImageOut(&p, *b++);
		for (y = 4; y < i->H; y += 8)
			for (b = &ImagePixel(i, 0, y), x = 0; x < i->W; x++)
				ImageOut(&p, *b++);
		for (y = 2; y < i->H; y += 4)
			for (b = &ImagePixel(i, 0, y), x = 0; x < i->W; x++)
				ImageOut(&p, *b++);
		for (y = 1; y < i->H; y += 2)
			for (b = &ImagePixel(i, 0, y), x = 0; x < i->W; x++)
				ImageOut(&p, *b++);
#else
		for (y = 0; y < i->H; y++)
			for (b = &ImagePixel(i, 0, y), x = 0; x < i->W; x++)
				ImageOut(&p, *b++);
#endif
		ImageEnd(&p);
	}
	write(fh, "\0", 1);	// end of image data
	write(fh, "\x3B", 1);	// trailer
}

void ImageText(Image * i, int x, int y, int col, char *text)
{				// writes 8x8 text
	if (i && text)
		while (*text) {
			if (*text >= ' ' && *text) {
				int r;
				unsigned const char *b =
				    bbc + (*text - ' ') * 8;
				for (r = 0; r < 8; r++) {
					unsigned char v = *b++;
					unsigned char *p =
					    &ImagePixel(i, x, y + r);
					unsigned char m;
					for (m = 0x80; m; m >>= 1, p++)
						if (v & m)
							*p = col;
				}
			}
			x += 8;
			text++;
		}
}

void ImageSmall(Image * i, int x, int y, int col, char *text)
{				// writes 4x6 digits
	if (i && text)
		while (*text) {
			char *p = strchr(smallc, toupper(*text));
			if (p) {
				int r;
				char m = 1;
				unsigned const char *b =
				    small + (p - smallc) * 3;
				for (r = 0; r < 5; r++) {
					int c;
					for (c = 0; c < 3; c++)
						if (b[c] & m)
							ImagePixel(i, x + c,
								   y + r) = col;
					m <<= 1;
				}
				x += 4;
			} else if (*text == '.') {
				ImagePixel(i, x, y + 4) = col;
				x += 2;
			} else if (*text == ':') {
				ImagePixel(i, x, y + 1) = col;
				ImagePixel(i, x, y + 3) = col;
				x += 2;
			}
			text++;
		}
}

void ImageRect(Image * i, int x, int y, int w, int h, int c)
{				// fill a box
	if (i && w && h) {
		while (h--) {
			unsigned char *p = &ImagePixel(i, x, y);
			int n = w;
			while (n--)
				*p++ = c;
			y++;
		}
	}
}

// PNG code

      /* Table of CRCs of all 8-bit messages. */
static unsigned int crc_table[256];

      /* Make the table for a fast CRC. */
void make_crc_table(void)
{
	unsigned int c;
	int n, k;
	for (n = 0; n < 256; n++) {
		c = (unsigned int)n;
		for (k = 0; k < 8; k++) {
			if (c & 1)
				c = 0xedb88320L ^ (c >> 1);
			else
				c = c >> 1;
		}
		crc_table[n] = c;
	}
}

      /* Update a running CRC with the bytes buf[0..len-1]--the CRC
         should be initialized to all 1's, and the transmitted value
         is the 1's complement of the final running CRC (see the
         crc() routine below)). */

unsigned int update_crc(unsigned int crc, unsigned char *buf, int len)
{
	unsigned int c = crc;
	int n;

	for (n = 0; n < len; n++)
		c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);

	return c;
}

      /* Return the CRC of the bytes buf[0..len-1]. */
unsigned int crc(unsigned char *buf, int len)
{
	return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
}

unsigned int writecrc(int fh, char *ptr, int len, unsigned int c)
{
	write(fh, ptr, len);
	while (len--)
		c = crc_table[(c ^ *ptr++) & 0xff] ^ (c >> 8);
	return c;
}

void writechunk(int fh, char *typ, void *ptr, int len)
{
	unsigned int v = htonl(len), crc;
	write(fh, &v, 4);
	crc = writecrc(fh, typ, 4, ~0);
	if (len)
		crc = writecrc(fh, ptr, len, crc);
	v = htonl(~crc);
	write(fh, &v, 4);
}

#ifndef USEZLIB
unsigned int adlersum(unsigned char *p, int l, unsigned int adler)
{
	unsigned int s1 = (adler & 65535), s2 = (adler >> 16);
	while (l--) {
		s1 += *p++;
		s2 += s1;
	}
	s1 %= 65521;		// can be delayed due to sensible "l" values...
	s2 %= 65521;
	return (s2 << 16) + s1;
}
#endif

// write PNG image
void ImageWritePNG(Image * i, int fh, int back, int trans, char *comment)
{
	make_crc_table();
	write(fh, "\211PNG\r\n\032\n", 8);	// PNG header
	{			// IHDR
		struct {
			unsigned int width;
			unsigned int height;
			unsigned char depth;
			unsigned char colour;
			unsigned char compress;
			unsigned char filter;
			unsigned char interlace;
		} ihdr = {
		0, 0, 8, 3, 0, 0};
		ihdr.width = htonl(i->W);
		ihdr.height = htonl(i->H);
		writechunk(fh, "IHDR", &ihdr, 13);
	}
	{			// PLTE
		unsigned int v = htonl(i->C * 3), crc, n;
		write(fh, &v, 4);
		crc = writecrc(fh, "PLTE", 4, ~0);
		for (n = 0; n < i->C; n++) {
			v = htonl(i->Colour[n] << 8);
			crc = writecrc(fh, (void *)&v, 3, crc);
		}
		v = htonl(~crc);
		write(fh, &v, 4);
	}
	if (back >= 0) {	// bKGD
		unsigned char b = back;
		writechunk(fh, "bKGD", &b, 1);
	}
	if (*comment) {		// tEXt
		char c[] = "Comment";
		unsigned int v = htonl(strlen(c) + strlen(comment) + 1), crc;
		write(fh, &v, 4);
		crc = writecrc(fh, "tEXt", 4, ~0);
		crc = writecrc(fh, c, strlen(c) + 1, crc);
		crc = writecrc(fh, comment, strlen(comment), crc);
		v = htonl(~crc);
		write(fh, &v, 4);
	}
	{			// tRNS
		unsigned char alpha[256];
		int n;
		for (n = 0; n < i->C; n++)
			// 4th palette byte treated as 0=opaque, 255-transparren
			alpha[n] = 255 - (i->Colour[n] >> 24);
		if (trans >= 0 && trans < i->C)
			// manual set of specific transparrent colour
			alpha[trans] = 0;
		writechunk(fh, "tRNS", alpha, i->C);
	}
#ifndef USEZLIB
	{			// IDAT
		unsigned int v = htonl(i->H * (i->L + 5) + 6),
		    crc, adler = 1, n;
		unsigned char *p = i->Image;
		write(fh, &v, 4);
		crc = writecrc(fh, "IDAT", 4, ~0);
		crc = writecrc(fh, "\170\001", 2, crc);	// zlib header for deflate
		n = i->H;
		while (n--) {
			unsigned char h[5];
			h[0] = (n ? 0 : 1);	// last chunk in deflate, un compressed
			h[1] = (i->L & 255);	// Len, LSB first as per deflate spec
			h[2] = (i->L / 256);
			h[3] = ~(i->L & 255);	// Inverse of Len
			h[4] = ~(i->L / 256);
			*p = 0;	// filter 0 (NONE)
			crc = writecrc(fh, h, 5, crc);
			crc = writecrc(fh, p, i->L, crc);
			adler = adlersum(p, i->L, adler);
			p += i->L;
		}
		v = htonl(adler);
		crc = writecrc(fh, (void *)&v, 4, crc);
		v = htonl(~crc);
		write(fh, &v, 4);
	}
#else
	{			// IDAT
		unsigned char *temp;
		unsigned long n;
		for (n = 0; n < i->H; n++)
			i->Image[n * i->L] = 0;	// filter 0
		n = i->H * i->L * 1001 / 1000 + 12;
		temp = malloc(n);
		if (compress2(temp, &n, i->Image, i->L * i->H, 9) != Z_OK)
			fprintf(stderr, "Deflate error\n");
		else
			writechunk(fh, "IDAT", temp, n);
		free(temp);
	}
#endif
	writechunk(fh, "IEND", 0, 0);	// IEND
}
