/*
 * msi.c -- encoding for MSI-Plessey
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


/* Patterns */
static char *patterns[] = { "13", "31" };

static char *fillers[] = { "031", "131" };

static int width = 16 /* each character uses 4 patterns */,
	startpos = 6 /* length of the first filler */;

/*
 * Check that the text can be encoded. Returns 0 or -1.
 */
int Barcode_msi_verify(unsigned char *text)
{
    int i;

    if (!strlen(text))
	return -1;
    for (i=0; text[i]; i++) {
        if (!isdigit(text[i]))
            return -1;
    }
    return 0;
}

static int add_one(char *ptr, int code)
{
    sprintf(ptr, "%s%s%s%s", 
	patterns[(code >> 3) & 1],
	patterns[(code >> 2) & 1],
	patterns[(code >> 1) & 1],
	patterns[code & 1]);
    return 0;
}

/*
 * The encoding functions fills the "partial" and "textinfo" fields.
 * Lowercase chars are converted to uppercase
 */
int Barcode_msi_encode(struct Barcode_Item *bc)
{
    static char *text;
    static char *partial;  /* dynamic */
    static char *textinfo; /* dynamic */
    char *ptr, *textptr;
    int i, code, textpos, usesum, checksum = 0;

    if (bc->partial)
	free(bc->partial);
    if (bc->textinfo)
	free(bc->textinfo);
    bc->partial = bc->textinfo = NULL; /* safe */

    if (!bc->encoding)
	bc->encoding = strdup("msi");

    if ((bc->flags & BARCODE_NO_CHECKSUM))
	usesum = 0;
    else
	usesum = 1;

    text = bc->ascii;

    /* the partial code is head + 8 * (text + check) + tail + margin + term. */
    partial = malloc( 3 + 8 * (strlen(text) + 1) + 3 + 2 );
    if (!partial) {
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
        code = text[i] - '0';
        add_one(ptr, code);
        sprintf(textptr, "%i:12:%c ", textpos, text[i]);
        
        textpos += width; /* width of each code */
        textptr += strlen(textptr);
        ptr += strlen(ptr); 
	if (usesum) {
	/* For a code ...FEDCBA the checksum is computed
	 * as the sum of digits of the number ...FDB plus
	 * the sum of digits of the number ...ECA * 2.
	 * Which is equivalent to the sum of each digit of ...ECA doubled
	 * plus carry.
	 */
	    if ((i ^ strlen(text)) & 1) {
		/* a last digit, 2 away from last, etc. */
		checksum += 2 * code + (2 * code) / 10;
	    } else {
		checksum += code;
	    }
	}
    }
    /* Some implementations use a double checksum. Currently the only way
     * to print a barcode with double checksum is to put the checksum
     * digit printed below at the end of a code in the command line
     * and re-run the program.
     */
    if (usesum) {
	/* the check digit is the complement of the checksum
	 * to a multiple of 10.
	 */
	checksum = (checksum + 9) / 10 * 10 - checksum;
	/* fprintf(stderr, "Checksum: %d\n", checksum); */
	add_one(ptr, checksum);
	ptr += strlen(ptr);
    }

    strcpy(ptr, fillers[1]);
    bc->partial = partial;
    bc->textinfo = textinfo;

    return 0;
}
