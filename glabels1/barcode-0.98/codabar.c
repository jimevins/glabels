/*
 * codabar.c -- encoding for Codabar
 *
 * Copyright (c) 2000 Leonid A. Broukhis (leob@mailcom.com)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "barcode.h"


/* this is ordered in decades to simplify encoding */
static char alphabet[] = 
   "0123456789" "-$:/.+ABCD";

#define CODE_A	16
#define CODE_B	17

#define NARROW	12
#define WIDE	14

/* Patterns */
static char *patterns[] = {
"1111133","1111331","1113113","3311111","1131131",
"3111131","1311113","1311311","1331111","3113111",
"1113311","1133111","3111313","3131113","3131311",
"1131313","1133131","1313113","1113133","1113331" };

/*
 * Check that the text can be encoded. Returns 0 or -1.
 * If it's all lowecase convert to uppercase and accept it.
 * If the first character is a letter (A to D), the last one must be too;
 * no other character should be a letter.
 */
int Barcode_cbr_verify(unsigned char *text)
{
    int i, lower=0, upper=0;
    int startpresent = 0;

    if (!strlen(text))
	return -1;
    for (i=0; text[i]; i++) {
	char * pos;
        if (isupper(text[i])) upper++;
        if (islower(text[i])) lower++;
	pos = strchr(alphabet,toupper(text[i]));
        if (!pos)
            return -1;
	if (i == 0 && pos - alphabet >= CODE_A)
	    startpresent = 1;
	else if (pos - alphabet >= CODE_A &&
		 (!startpresent || i != strlen(text) - 1))
	    return -1; 	
    }
    if (lower && upper)
        return -1;
    return 0;
}

static int add_one(char *ptr, int code)
{
    sprintf(ptr,"1%s", /* separator */ patterns[code]);
    return 0;
}

/*
 * The encoding functions fills the "partial" and "textinfo" fields.
 * Lowercase chars are converted to uppercase
 */
int Barcode_cbr_encode(struct Barcode_Item *bc)
{
    static char *text;
    static char *partial;  /* dynamic */
    static char *textinfo; /* dynamic */
    char *c, *ptr, *textptr;
    int i, code, textpos, usesum, checksum = 0, startpresent;

    if (bc->partial)
	free(bc->partial);
    if (bc->textinfo)
	free(bc->textinfo);
    bc->partial = bc->textinfo = NULL; /* safe */

    if (!bc->encoding)
	bc->encoding = strdup("codabar");

    text = bc->ascii;
    if (!text) {
        bc->error = EINVAL;
        return -1;
    }
    /* the partial code is 8 * (head + text + check + tail) + margin + term. */
    partial = malloc( (strlen(text) + 3) * 8 + 2);
    if (!partial) {
        bc->error = errno;
        return -1;
    }

    /* the text information is at most "nnn:fff:c " * (strlen + check) +term */
    textinfo = malloc(10*(strlen(text) + 1) + 2);
    if (!textinfo) {
        bc->error = errno;
        free(partial);
        return -1;
    }

    ptr = partial;
    textptr = textinfo;
    textpos = 0;
    usesum = bc->flags & BARCODE_NO_CHECKSUM ? 0 : 1;
    /* if no start character specified, A is used as a start character */
    if (!isalpha(text[0])) {
	add_one(ptr, CODE_A);
	ptr += strlen(ptr);
	textpos = WIDE;
	checksum = CODE_A;
	startpresent = 0;
    } else {
	startpresent = 1;
    }
    for (i=0; i<strlen(text); i++) {
        c = strchr(alphabet, toupper(text[i]));
        if (!c) {
            bc->error = EINVAL; /* impossible if text is verified */
            free(partial);
            free(textinfo);
            return -1;
        }
        code = c - alphabet;
        add_one(ptr, code);
        sprintf(textptr, "%i:12:%c ", textpos, toupper(text[i]));
        textpos += code < 12 ? NARROW : WIDE;
        textptr += strlen(textptr);
        ptr += strlen(ptr); 
	checksum += code;
	if (startpresent && usesum && i == strlen(text) - 2) {
	    /* stuff a check symbol before the stop */
	    c = strchr(alphabet, toupper(text[i+1]));
	    if (!c) /* impossible */
		continue;
	    code = c - alphabet;
	    checksum += code;

	    /* Complement to a multiple of 16 */
	    checksum = (checksum + 15) / 16 * 16 - checksum;
	    add_one(ptr, checksum);
	    ptr += strlen(ptr);
	}
    }
    if (!startpresent) {
	if (usesum) {
	    /* if no start character specified, B is used as a stop char */
	    checksum += CODE_B;
	    checksum = (checksum + 15) / 16 * 16 - checksum;
	    add_one(ptr, checksum);
	    ptr += strlen(ptr);
	}
	add_one(ptr, CODE_B);
    }
    bc->partial = partial;
    bc->textinfo = textinfo;

    return 0;
}
