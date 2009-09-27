/*
 * plessey.c -- encoding for Plessey
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

static char * patterns[] = { "13", "31" };

/* this is ordered in decades to simplify encoding */
static char alphabet[] = 
   "0123456789" "ABCDEF";

/* stop sequence may be 231311313 (barcodemill.com) */
static char *fillers[] = { "031311331", "331311313" };

static int width = 16, startpos = 16;

/*
 * Check that the text can be encoded. Returns 0 or -1.
 * If it's all lowecase convert to uppercase and accept it
 */
int Barcode_pls_verify(unsigned char *text)
{
    int i, upper = 0, lower = 0;

    if (!strlen(text))
	return -1;
    for (i=0; text[i]; i++) {
        if (!strchr(alphabet,toupper(text[i])))
            return -1;
	if (isupper(text[i])) upper++;
	if (islower(text[i])) lower++;
    }
    if (upper && lower)
	return -1;
    return 0;
}

static int add_one(char *ptr, int code)
{
    sprintf(ptr, "%s%s%s%s", 
	patterns[code & 1],
	patterns[(code >> 1) & 1],
	patterns[(code >> 2) & 1],
	patterns[(code >> 3) & 1]
    );
    return 0;
}

/*
 * The encoding functions fills the "partial" and "textinfo" fields.
 * Lowercase chars are converted to uppercase
 */
int Barcode_pls_encode(struct Barcode_Item *bc)
{
    static char *text;
    static char *partial;  /* dynamic */
    static char *textinfo; /* dynamic */
    char *c, *ptr, *textptr;
    unsigned char *checkptr;
    int i, code, textpos;
    static char check[9] = {1,1,1,1,0,1,0,0,1};
    if (bc->partial)
	free(bc->partial);
    if (bc->textinfo)
	free(bc->textinfo);
    bc->partial = bc->textinfo = NULL; /* safe */

    if (!bc->encoding)
	bc->encoding = strdup("plessey");

    text = bc->ascii;
    if (!text) {
        bc->error = EINVAL;
        return -1;
    }
    /* the partial code is 8 * (head + text + check + tail) + margin + term. */
    partial = malloc( (strlen(text) + 4) * 8 + 3);
    checkptr = calloc (1, strlen(text) * 4 + 8);

    if (!partial || !checkptr) {
	if (partial) free(partial);
	if (checkptr) free(checkptr);
        bc->error = errno;
        return -1;
    }

    /* the text information is at most "nnn:fff:c " * strlen +term */
    textinfo = malloc(10*strlen(text) + 2);
    if (!textinfo) {
        bc->error = errno;
        free(partial);
        return -1;
    }

    strcpy(partial, fillers[0]);
    ptr = partial + strlen(partial);
    textptr = textinfo;
    textpos = startpos;
    
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
        
        textpos += width; /* width of each code */
        textptr += strlen(textptr);
        ptr += strlen(ptr); 
	checkptr[4*i] = code & 1;
	checkptr[4*i+1] = (code >> 1) & 1;
	checkptr[4*i+2] = (code >> 2) & 1;
	checkptr[4*i+3] = (code >> 3) & 1;
    }
    /* The CRC checksum is required */
    for (i=0; i < 4*strlen(text); i++) {
        int j;
        if (checkptr[i])
                for (j = 0; j < 9; j++)
                        checkptr[i+j] ^= check[j];
    }
    for (i = 0; i < 8; i++) {
        sprintf(ptr, patterns[checkptr[strlen(text) * 4 + i]]);
	ptr += 2;
    }
    fprintf(stderr, "CRC: ");
    for (i = 0; i < 8; i++) {
        fputc('0' + checkptr[strlen(text) * 4 + i], stderr);
    }
    fputc('\n', stderr);
    strcpy(ptr, fillers[1]);
    bc->partial = partial;
    bc->textinfo = textinfo;

    return 0;
}
