/*
 * barcode.h -- definitions for libbarcode
 *
 * Copyright (c) 1999 Alessandro Rubini (rubini@gnu.org)
 * Copyright (c) 1999 Prosa Srl. (prosa@prosa.it)
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

#ifndef _BARCODE_H_
#define _BARCODE_H_

#include <stdio.h>

#define BARCODE_VERSION     "0.98" 
#define BARCODE_VERSION_INT   9800

/*
 * The object
 */
struct Barcode_Item {
    int flags;         /* type of encoding and decoding */
    char *ascii;       /* malloced */
    char *partial;     /* malloced too */
    char *textinfo;    /* information about text positioning */
    char *encoding;    /* code name, filled by encoding engine */
    int width, height; /* output units */
    int xoff, yoff;    /* output units */
    int margin;        /* output units */
    double scalef;     /* requested scaling for barcode */
    int error;         /* an errno-like value, in case of failure */
};

/*
 * The flags field
 */
#define BARCODE_DEFAULT_FLAGS 0x00000000

#define BARCODE_ENCODING_MASK 0x000000ff   /* 256 possibilites... */
#define BARCODE_NO_ASCII      0x00000100   /* avoid text in output */
#define BARCODE_NO_CHECKSUM   0x00000200   /* avoid checksum in output */

#define BARCODE_OUTPUT_MASK   0x000ff000   /* 256 output types */
#define BARCODE_OUT_EPS       0x00001000
#define BARCODE_OUT_PS        0x00002000
#define BARCODE_OUT_PCL       0x00004000   /* by Andrea Scopece */
/*                  PCL_III   0x00008000   */
#define BARCODE_OUT_PCL_III   0x0000C000
#define BARCODE_OUT_NOHEADERS 0x00100000   /* no header nor footer */

enum {
    BARCODE_ANY = 0,     /* choose best-fit */
    BARCODE_EAN,
    BARCODE_UPC,        /* upc == 12-digit ean */
    BARCODE_ISBN,       /* isbn numbers (still EAN13) */
    BARCODE_39,         /* code 39 */
    BARCODE_128,        /* code 128 (a,b,c: autoselection) */
    BARCODE_128C,       /* code 128 (compact form for digits) */
    BARCODE_128B,       /* code 128, full printable ascii */
    BARCODE_I25,        /* interleaved 2 of 5 (only digits) */
    BARCODE_128RAW,     /* Raw code 128 (by Leonid A. Broukhis) */
    BARCODE_CBR,        /* Codabar (by Leonid A. Broukhis) */
    BARCODE_MSI,        /* MSI (by Leonid A. Broukhis) */
    BARCODE_PLS,        /* Plessey (by Leonid A. Broukhis) */
    BARCODE_93          /* code 93 (by Nathan D. Holmes) */
};

#define BARCODE_DEFAULT_MARGIN 10

#ifdef  __cplusplus
extern "C" {
#endif
/*
 * Create and destroy barcode structures
 */
extern struct Barcode_Item *Barcode_Create(char *text);
extern int                  Barcode_Delete(struct Barcode_Item *bc);

/*
 * Encode and print
 */
extern int Barcode_Encode(struct Barcode_Item *bc, int flags);
extern int Barcode_Print(struct Barcode_Item *bc, FILE *f, int flags);

/*
 * Choose the position
 */
extern int Barcode_Position(struct Barcode_Item *bc, int wid, int hei,
			    int xoff, int yoff, double scalef);

/*
 * Do it all in one step
 */
extern int Barcode_Encode_and_Print(char *text, FILE *f, int wid, int hei,
				    int xoff, int yoff, int flags);


/*
 * Return current version (integer and string)
 */
extern int Barcode_Version(char *versionname);

#ifdef  __cplusplus
}
#endif

#endif /* _BARCODE_H_ */
