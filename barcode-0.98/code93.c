/*
 * code39.c -- encoding for code39
 *
 * Copyright (c) 1999 Alessandro Rubini (rubini@gnu.org) 
 * Copyright (c) 1999 Prosa Srl. (prosa@prosa.it)
 * Copyright (c) 2001 Nathan D. Holmes / FedEx Services (ndholmes@fedex.com) 
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
   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%";

static char *codeset[] = {
	"131112", "111213", "111312", "111411", "121113",  /*  0  -  4 */
	"121212", "121311", "111114", "131211", "141111",  /*  5  -  9 */
	"211113", "211212", "211311", "221112", "221211",  /*  A  -  E */
	"231111", "112113", "112212", "112311", "122112",  /*  F  -  J */
	"132111", "111123", "111222", "111321", "121122",  /*  K  -  O */
	"131121", "212112", "212211", "211122", "211221",  /*  P  -  T */
	"221121", "222111", "112122", "112221", "122121",  /*  U  -  Y */
	"123111", "121131", "311112", "311211", "321111",  /*  Z  -  $ */
	"112131", "113121", "211131", "121221", "312111",  /*  /  - (%) */
	"311121", "122211", "111141"                       /* (/) -  (terminator) */
};


/*      00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001111111111111111111111111111 */
/*      00000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999999990000000000111111111122222222 */
/*      01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567 */
static char shiftset[] =
       "%$$$$$$$$$$$$$$$$$$$$$$$$$$%%%%% ///  ///// /             /%%%%%%                          %%%%%%++++++++++++++++++++++++++%%%%%";
static char shiftset2[] =
       "UABCDEFGHIJKLMNOPQRSTUVWXYZABCDE ABC  FGHIJ L             ZFGHIJV                          KLMNOWABCDEFGHIJLKMNOPQRSTUVWXYZPQRST";

#define START_STOP 47
#define EXTEND_DOLLAR   43
#define EXTEND_PERCENT   44
#define EXTEND_SLASH   45
#define EXTEND_PLUS   46

/*
 * Check that the text can be encoded. Returns 0 or -1.
 * All of ASCII-7 is accepted.
 */
int Barcode_93_verify(unsigned char *text)
{
    int i;

    if (!strlen(text))
	return -1;
    for (i=0; text[i]; i++) {

        /* Test if beyond the range of 7-bit ASCII */
        if ((int)text[i] > 127)
            return -1;
    }
    return 0;
}

/*
 * The encoding functions fills the "partial" and "textinfo" fields.
 * Lowercase chars are converted to uppercase
 */
int Barcode_93_encode(struct Barcode_Item *bc)
{
    static unsigned char *text;
    static char *partial;  /* dynamic */
    static char *textinfo; /* dynamic */
    char *c, *textptr;
    int *checksum_str;
    int i, code, textpos, checksum_len=0;
    int c_checksum = 0;
    int k_checksum = 0;

    if (bc->partial)
	free(bc->partial);
    if (bc->textinfo)
	free(bc->textinfo);
    bc->partial = bc->textinfo = NULL; /* safe */

    if (!bc->encoding)
	bc->encoding = strdup("code 93");

    text = bc->ascii;
    if (!text) {
        bc->error = EINVAL;
        return -1;
    }

    /* the partial code is 6 * (head + 2* text + 2* check + tail) + margin + term. */
    partial = malloc( (strlen(text) *2 + 4) * 6 +2);
    if (!partial) {
        bc->error = errno;
        return -1;
    }

    checksum_str = (int *)malloc( (strlen(text) + 3) * 2 * sizeof(int));
    if (!checksum_str) {
	free(partial);
        bc->error = errno;
        return -1;
    }

    /* the text information is at most "nnn:fff:c " * strlen +term */
    textinfo = malloc(10*strlen(text) + 2);
    if (!textinfo) {
        bc->error = errno;
        free(partial);
	free(checksum_str);
        return -1;
    }

    textptr = textinfo;
    textpos = 22;
    strcpy(partial, "0");
    strcat(partial, codeset[START_STOP]);

    for (i=0; i<strlen(text); i++) {
        c = strchr(alphabet, text[i]);
        if (!c) {
           /* Encode the shift character */
	   switch(shiftset[text[i]])
           {
              case '$':
                 code = EXTEND_DOLLAR;
                 break;
              case '%':
                 code = EXTEND_PERCENT;
                 break;
              case '/':
                 code = EXTEND_SLASH;
                 break;
              case '+':
                 code = EXTEND_PLUS;
                 break;
              default:
                 code = 0;
                 break;
           }

           strcat(partial, codeset[code]);
           checksum_str[checksum_len++] = code;

           /* Encode the second character */
           code = strchr(alphabet, shiftset2[text[i]]) - alphabet;
           strcat(partial, codeset[code]);
           checksum_str[checksum_len++] = code;
        } else {
            /* Character is natively supported by the symbology, encode directly */
            code = c - alphabet;
            strcat(partial, codeset[c -alphabet]);
            checksum_str[checksum_len++] = code;
        }
        sprintf(textptr, "%i:12:%c ", textpos, text[i]);
        textptr += strlen(textptr);
	/*
	 * width of each code is 9; composed codes are 18, but aligning
	 * the text behind the right bars would be ugly, so use 9 anyways
	 */
        textpos += 9;
    }
    c_checksum=0;
    k_checksum=0;

    /* Add the checksum */
    if ( (bc->flags & BARCODE_NO_CHECKSUM)==0 ) 
    {
        for(i=1; i<=checksum_len; i++)
        {
           c_checksum += i * (int)checksum_str[checksum_len - i];
           k_checksum += (i+1) * (int)checksum_str[checksum_len - i];
        }

        c_checksum = c_checksum % 47;
        k_checksum += c_checksum;                           
        k_checksum = k_checksum % 47;

        strcat(partial, codeset[c_checksum]);
        strcat(partial, codeset[k_checksum]);

    }
    strcat(partial, codeset[START_STOP]); /* end */

    /* Encodes final 1-unit width bar to turn the start character into a stop char */
    strcat(partial, "1");
    bc->partial = partial;
    bc->textinfo = textinfo;

    return 0;
}
