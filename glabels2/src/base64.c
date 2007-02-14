/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  base64.c:  GLabels base64 encode/decode module
 *
 *  Copyright (C)  2003  Jim Evins <evins@snaught.com>
 *
 *  This module is based on base64.c from fetchmail:
 *
 *  Copyright (C)2002 by Eric S. Raymond.
 *  Portions are copyrighted by Carl E. Harris and George M. Sipe.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

/*
 * This base 64 encoding is defined in RFC2045 section 6.8.
 */
#include <config.h>

#include "base64.h"

#include <glib/gmem.h>
#include <string.h>

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

#define LINE_LENGTH 76 /* Must be <= 76 and must be a multiple of 4 */
#define BAD	-1

#define DECODE64(c)  (isascii(c) ? base64val[c] : BAD)

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static const gchar base64digits[] =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const gchar base64val[] = {
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD, 62, BAD,BAD,BAD, 63,
     52, 53, 54, 55,  56, 57, 58, 59,  60, 61,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,  0,  1,  2,   3,  4,  5,  6,   7,  8,  9, 10,  11, 12, 13, 14,
     15, 16, 17, 18,  19, 20, 21, 22,  23, 24, 25,BAD, BAD,BAD,BAD,BAD,
    BAD, 26, 27, 28,  29, 30, 31, 32,  33, 34, 35, 36,  37, 38, 39, 40,
     41, 42, 43, 44,  45, 46, 47, 48,  49, 50, 51,BAD, BAD,BAD,BAD,BAD
};

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/


/*****************************************************************************/
/* Encode to Base64 string.                                                  */
/*****************************************************************************/
gchar *
gl_base64_encode (const guchar *in, guint inlen)
{
	gchar *out, *p_out;
	gint   buf_size;
	gint   i;

        /* Calculate output buffer size */
	buf_size  = 4*((inlen+2)/3);            /* Encoded characters */
	buf_size += buf_size / LINE_LENGTH + 2; /* Line breaks */
	buf_size += 1;                          /* null termination */
	
	/* Allocate output buffer */
	out = g_new0 (gchar, buf_size);
	p_out=out;

	/* Now do the encoding */
	*p_out++ = '\n';
	for ( i=0; inlen >= 3; inlen-=3 ) {

		*p_out++ = base64digits[in[0] >> 2];
		*p_out++ = base64digits[((in[0] << 4) & 0x30) | (in[1] >> 4)];
		*p_out++ = base64digits[((in[1] << 2) & 0x3c) | (in[2] >> 6)];
		*p_out++ = base64digits[in[2] & 0x3f];
		in += 3;

		i += 4;
		if ( (i % LINE_LENGTH) == 0 ) {
			*p_out++ = '\n';
		}

	}
	if (inlen > 0) {
		guchar fragment;
    
		*p_out++ = base64digits[in[0] >> 2];
		fragment = (in[0] << 4) & 0x30;
		if (inlen > 1)
			fragment |= in[1] >> 4;
		*p_out++ = base64digits[fragment];
		*p_out++ = (inlen < 2) ? '=' : base64digits[(in[1] << 2) & 0x3c];
		*p_out++ = '=';

		*p_out++ = '\n';
	}
	*p_out++ = '\0';

	return out;
}

/*****************************************************************************/
/* Decode from a Base64 string.                                              */
/*****************************************************************************/
guchar *
gl_base64_decode (const gchar *in, guint *outlen)
{
	gchar           *out, *p_out;
	gint             buf_size;
	register guchar  digit1, digit2, digit3, digit4;

        /* Calculate output buffer size */
	buf_size = strlen (in) * 3 / 4;

	/* Allocate output buffer */
	out = g_new0 (gchar, buf_size);

	*outlen = 0;
	p_out = out;

	/* Skip non-printable characters */
	while ( (*in == '\n') || (*in == '\r') || (*in == ' ') ) {
		in ++;
	}
	if (!*in) {
		g_free (out);
		return NULL;
	}

	/* Now do the decoding */
	do {
		digit1 = in[0];
		if (DECODE64(digit1) == BAD) {
			g_free (out);
			return NULL;
		}
		digit2 = in[1];
		if (DECODE64(digit2) == BAD) {
			g_free (out);
			return NULL;
		}
		digit3 = in[2];
		if (digit3 != '=' && DECODE64(digit3) == BAD) {
			g_free (out);
			return NULL;
		}
		digit4 = in[3];
		if (digit4 != '=' && DECODE64(digit4) == BAD) {
			g_free (out);
			return NULL;
		}
		in += 4;

		*p_out++ = (DECODE64(digit1)<<2) | (DECODE64(digit2) >> 4);
		(*outlen)++;
		if (digit3 != '=')
		{
			*p_out++ = ((DECODE64(digit2)<<4)&0xf0) | (DECODE64(digit3)>>2);
			(*outlen)++;
			if (digit4 != '=')
			{
				*p_out++ = ((DECODE64(digit3)<<6)&0xc0) | DECODE64(digit4);
				(*outlen)++;
			}
		}

		/* Skip non-printable characters */
		while ( (*in == '\n') || (*in == '\r') || (*in == ' ') ) {
			in ++;
		}

	} while (*in && digit4 != '=');

	return (guchar *)out;
}

