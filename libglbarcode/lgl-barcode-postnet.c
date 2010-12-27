/*
 *  lgl-barcode-postnet.c
 *  Copyright (C) 2001-2010  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of libglbarcode.
 *
 *  libglbarcode is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libglbarcode is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libglbarcode.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This module implements the POSTNET barcode specified in the USPS
 * publication 25, Mar 2001.
 */

#include <config.h>

#include "lgl-barcode-postnet.h"

#include <glib.h>
#include <ctype.h>


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

#define PTS_PER_INCH 72.0

#define POSTNET_BAR_WIDTH      ( 0.02    * PTS_PER_INCH )
#define POSTNET_FULLBAR_HEIGHT ( 0.125   * PTS_PER_INCH )
#define POSTNET_HALFBAR_HEIGHT ( 0.05    * PTS_PER_INCH )
#define POSTNET_BAR_PITCH      ( 0.04545 * PTS_PER_INCH )
#define POSTNET_HORIZ_MARGIN   ( 0.125   * PTS_PER_INCH )
#define POSTNET_VERT_MARGIN    ( 0.04    * PTS_PER_INCH )


/*===========================================*/
/* Private globals                           */
/*===========================================*/
static gchar *symbols[] = {
        /* 0 */ "11000",
        /* 1 */ "00011",
        /* 2 */ "00101",
        /* 3 */ "00110",
        /* 4 */ "01001",
        /* 5 */ "01010",
        /* 6 */ "01100",
        /* 7 */ "10001",
        /* 8 */ "10010",
        /* 9 */ "10100",
};

static gchar *frame_symbol = "1";


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/
static gint         postnet_validate_data (const gchar *data);

static gchar       *postnet_encode        (const gchar *digits);

static lglBarcode  *postnet_vectorize     (const gchar *code);



/****************************************************************************/
/* Generate new Postnet barcode structure from data.                        */
/****************************************************************************/
lglBarcode *
lgl_barcode_postnet_new (lglBarcodeType  type,
                         gboolean        text_flag,
                         gboolean        checksum_flag,
                         gdouble         w,
                         gdouble         h,
                         const gchar    *data)
{
        gint                n_digits;
        gchar              *code;
        lglBarcode         *bc;

        /* Validate data and length for all subtypes. */
        n_digits = postnet_validate_data (data);
        switch (type)
        {

        case LGL_BARCODE_TYPE_POSTNET:
                if ( (n_digits !=  5) &&
                     (n_digits !=  9) &&
                     (n_digits != 11) )
                {
                        return NULL;
                }
                break;

        case LGL_BARCODE_TYPE_POSTNET_5:
                if ( n_digits != 5 )
                {
                        return NULL;
                }
                break;

        case LGL_BARCODE_TYPE_POSTNET_9:
                if ( n_digits != 9 )
                {
                        return NULL;
                }
                break;

        case LGL_BARCODE_TYPE_POSTNET_11:
                if ( n_digits != 11 )
                {
                        return NULL;
                }
                break;

        case LGL_BARCODE_TYPE_CEPNET:
                if ( n_digits !=  8 )
                {
                        return NULL;
                }
                break;

        default:
                g_message ("Invalid barcode type for POSTNET backend.");
                return NULL;

        }

        /* First get code string */
        code = postnet_encode (data);
        if (code == NULL)
        {
                return NULL;
        }

        /* Now vectorize encoded data */
        bc = postnet_vectorize (code);

        g_free (code);

        return bc;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Validate data, returning number of digits if valid.            */
/*--------------------------------------------------------------------------*/
static gint
postnet_validate_data (const gchar *data)
{
        gchar *p;
        gint   i;

        if (!data)
        {
                return 0;
        }

        for ( p = (gchar *)data, i=0; *p != 0; p++ )
        {
                if (g_ascii_isdigit (*p))
                {
                        i++;
                }
                else if ( (*p != '-') && (*p != ' ') )
                {
                        /* Only allow digits, dashes, and spaces. */
                        return 0;
                }
        }

        return i;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Generate string of symbols, representing barcode.              */
/*--------------------------------------------------------------------------*/
static gchar *
postnet_encode (const gchar *data)
{
        gchar   *p;
        gint     len;
        gint     d, sum;
        GString *code;

        /* Left frame bar */
        code = g_string_new (frame_symbol);

        sum = 0;
        for ( p = (gchar *)data, len = 0; (*p != 0) && (len < 11); p++ )
        {
                if (g_ascii_isdigit (*p))
                {
                        /* Only translate the digits (0-9) */
                        d = (*p) - '0';
                        sum += d;
                        code = g_string_append (code, symbols[d]);
                        len++;
                }
        }

        /* Create correction character */
        d = (10 - (sum % 10)) % 10;
        code = g_string_append (code, symbols[d]);

        /* Right frame bar */
        code = g_string_append (code, frame_symbol);

        return g_string_free (code, FALSE);
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Vectorize encoded barcode.                                     */
/*--------------------------------------------------------------------------*/
static lglBarcode *
postnet_vectorize (const gchar *code)
{
        lglBarcode         *bc;
        gchar              *p;
        gdouble             x, y, length, width;

        bc = lgl_barcode_new ();

        /* Now traverse the code string and create a list of lines */
        x = POSTNET_HORIZ_MARGIN;
        for ( p = (gchar *)code; *p != 0; p++ )
        {
                y = POSTNET_VERT_MARGIN;
                switch (*p)
                {
                case '0':
                        y += POSTNET_FULLBAR_HEIGHT - POSTNET_HALFBAR_HEIGHT;
                        length = POSTNET_HALFBAR_HEIGHT;
                        break;
                case '1':
                        length = POSTNET_FULLBAR_HEIGHT;
                        break;
                default:
                        break;
                }
                width = POSTNET_BAR_WIDTH;

                lgl_barcode_add_box (bc, x, y, width, length);

                x += POSTNET_BAR_PITCH;
        }

        bc->width = x + POSTNET_HORIZ_MARGIN;
        bc->height = POSTNET_FULLBAR_HEIGHT + 2 * POSTNET_VERT_MARGIN;

        return bc;
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
