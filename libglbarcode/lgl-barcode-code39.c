/*
 *  lgl-barcode-code39.c
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

#include <config.h>

#include "lgl-barcode-code39.h"

#include <glib.h>
#include <ctype.h>
#include <string.h>


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

#define PTS_PER_INCH 72.0

#define MIN_X        ( 0.01 *  PTS_PER_INCH )
#define N            2.5
#define MIN_I        MIN_X
#define MIN_HEIGHT   ( 0.25 *  PTS_PER_INCH )
#define MIN_QUIET    ( 0.10 *  PTS_PER_INCH )

#define INK_BLEED    ( 0.00325 * PTS_PER_INCH )

#define TEXT_AREA_HEIGHT 14.0
#define TEXT_SIZE        10.0


/*========================================================*/
/* Private types.                                         */
/*========================================================*/


/*===========================================*/
/* Private globals                           */
/*===========================================*/

/* Code 39 alphabet. Position indicates value. */
static gchar *alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%";

/* Code 39 symbols. Position must match position in alphabet. */
static gchar* symbols[43] = {
        /*        BsBsBsBsB */
        /* 0 */  "NnNwWnWnN",
        /* 1 */  "WnNwNnNnW",
        /* 2 */  "NnWwNnNnW",
        /* 3 */  "WnWwNnNnN",
        /* 4 */  "NnNwWnNnW",
        /* 5 */  "WnNwWnNnN",
        /* 6 */  "NnWwWnNnN",
        /* 7 */  "NnNwNnWnW",
        /* 8 */  "WnNwNnWnN",
        /* 9 */  "NnWwNnWnN",
        /* A */  "WnNnNwNnW",
        /* B */  "NnWnNwNnW",
        /* C */  "WnWnNwNnN",
        /* D */  "NnNnWwNnW",
        /* E */  "WnNnWwNnN",
        /* F */  "NnWnWwNnN",
        /* G */  "NnNnNwWnW",
        /* H */  "WnNnNwWnN",
        /* I */  "NnWnNwWnN",
        /* J */  "NnNnWwWnN",
        /* K */  "WnNnNnNwW",
        /* L */  "NnWnNnNwW",
        /* M */  "WnWnNnNwN",
        /* N */  "NnNnWnNwW",
        /* O */  "WnNnWnNwN",
        /* P */  "NnWnWnNwN",
        /* Q */  "NnNnNnWwW",
        /* R */  "WnNnNnWwN",
        /* S */  "NnWnNnWwN",
        /* T */  "NnNnWnWwN",
        /* U */  "WwNnNnNnW",
        /* V */  "NwWnNnNnW",
        /* W */  "WwWnNnNnN",
        /* X */  "NwNnWnNnW",
        /* Y */  "WwNnWnNnN",
        /* Z */  "NwWnWnNnN",
        /* - */  "NwNnNnWnW",
        /* . */  "WwNnNnWnN",
        /*   */  "NwWnNnWnN",
        /* $ */  "NwNwNwNnN",
        /* / */  "NwNwNnNwN",
        /* + */  "NwNnNwNwN",
        /* % */  "NnNwNwNwN",
};

static gchar *frame_symbol = "NwNnWnWnN";

static gchar *ascii_map[128] =
{
        /* NUL */ "%U",   /* SOH */ "$A",   /* STX */ "$B",   /* ETX */ "$C",
        /* EOT */ "$D",   /* ENQ */ "$E",   /* ACK */ "$F",   /* BEL */ "$G",
        /* BS  */ "$H",   /* HT  */ "$I",   /* LF  */ "$J",   /* VT  */ "$K",
        /* FF  */ "$L",   /* CR  */ "$M",   /* SO  */ "$N",   /* SI  */ "$O",
        /* DLE */ "$P",   /* DC1 */ "$Q",   /* DC2 */ "$R",   /* DC3 */ "$S",
        /* DC4 */ "$T",   /* NAK */ "$U",   /* SYN */ "$V",   /* ETB */ "$W",
        /* CAN */ "$X",   /* EM  */ "$Y",   /* SUB */ "$Z",   /* ESC */ "%A",
        /* FS  */ "%B",   /* GS  */ "%C",   /* RS  */ "%D",   /* US  */ "%E",
        /* " " */ " ",    /* !   */ "/A",   /* "   */ "/B",   /* #   */ "/C",
        /* $   */ "/D",   /* %   */ "/E",   /* &   */ "/F",   /* '   */ "/G",
        /* (   */ "/H",   /* )   */ "/I",   /* *   */ "/J",   /* +   */ "/K",
        /* ,   */ "/L",   /* -   */ "-",    /* .   */ ".",    /* /   */ "/O",
        /* 0   */ "0",    /* 1   */ "1",    /* 2   */ "2",    /* 3   */ "3",
        /* 4   */ "4",    /* 5   */ "5",    /* 6   */ "6",    /* 7   */ "7",
        /* 8   */ "8",    /* 9   */ "9",    /* :   */ "/Z",   /* ;   */ "%F",
        /* <   */ "%G",   /* =   */ "%H",   /* >   */ "%I",   /* ?   */ "%J",
        /* @   */ "%V",   /* A   */ "A",    /* B   */ "B",    /* C   */ "C",
        /* D   */ "D",    /* E   */ "E",    /* F   */ "F",    /* G   */ "G",
        /* H   */ "H",    /* I   */ "I",    /* J   */ "J",    /* K   */ "K",
        /* L   */ "L",    /* M   */ "M",    /* N   */ "N",    /* O   */ "O",
        /* P   */ "P",    /* Q   */ "Q",    /* R   */ "R",    /* S   */ "S",
        /* T   */ "T",    /* U   */ "U",    /* V   */ "V",    /* W   */ "W",
        /* X   */ "X",    /* Y   */ "Y",    /* Z   */ "Z",    /* [   */ "%K",
        /* \   */ "%L",   /* ]   */ "%M",   /* ^   */ "%N",   /* _   */ "%O",
        /* `   */ "%W",   /* a   */ "+A",   /* b   */ "+B",   /* c   */ "+C",
        /* d   */ "+D",   /* e   */ "+E",   /* f   */ "+F",   /* g   */ "+G",
        /* h   */ "+H",   /* i   */ "+I",   /* j   */ "+J",   /* k   */ "+K",
        /* l   */ "+L",   /* m   */ "+M",   /* n   */ "+N",   /* o   */ "+O",
        /* p   */ "+P",   /* q   */ "+Q",   /* r   */ "+R",   /* s   */ "+S",
        /* t   */ "+T",   /* u   */ "+U",   /* v   */ "+V",   /* w   */ "+W",
        /* x   */ "+X",   /* y   */ "+Y",   /* z   */ "+Z",   /* {   */ "%P",
        /* |   */ "%Q",   /* }   */ "%R",   /* ~   */ "%S",   /* DEL */ "%T" 
};


/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static gboolean    code39_is_data_valid     (const gchar *data);
static gboolean    code39_ext_is_data_valid (const gchar *data);

static gchar      *code39_encode            (const gchar *data,
                                             gboolean     checksum_flag);

static lglBarcode *code39_vectorize         (const gchar *code,
                                             gdouble      w,
                                             gdouble      h,
                                             gboolean     text_flag,
                                             gboolean     checksum_flag,
                                             const gchar *data,
                                             const gchar *string);


/****************************************************************************/
/* Generate new Code 39 barcode structure from data.                        */
/****************************************************************************/
lglBarcode *
lgl_barcode_code39_new (lglBarcodeType  type,
                        gboolean        text_flag,
                        gboolean        checksum_flag,
                        gdouble         w,
                        gdouble         h,
                        const gchar    *data)
{
        gchar         *canon_data;
        gchar         *display_data;
        gchar         *code, *p;
        lglBarcode    *bc;

        if ( (type != LGL_BARCODE_TYPE_CODE39) &&
             (type != LGL_BARCODE_TYPE_CODE39_EXT) )
        {
                g_message ("Invalid barcode type for CODE39 backend.");
                return NULL;
        }


        /* Validate data. */
        if (type == LGL_BARCODE_TYPE_CODE39)
        {
                if ( !code39_is_data_valid (data) )
                {
                        return NULL;
                }
                canon_data = g_ascii_strup (data, -1);
                display_data = g_strdup (canon_data);
        }
        else
        {
                GString *canon_data_str;

                if ( !code39_ext_is_data_valid (data) )
                {
                        return NULL;
                }

                canon_data_str = g_string_new ("");
                for ( p = (gchar *)data; *p != '\0'; p++ )
                {
                        canon_data_str = g_string_append (canon_data_str, ascii_map[(int)*p]);
                }
                canon_data   = g_string_free (canon_data_str, FALSE);

                display_data = g_strdup (data);
        }

        /* First get code string */
        code = code39_encode (canon_data, checksum_flag);
        if (code == NULL)
        {
                g_free (canon_data);
                g_free (display_data);
                return NULL;
        }

        /* Now vectorize code string */
        bc = code39_vectorize (code, w, h, text_flag, checksum_flag, canon_data, display_data);

        g_free (canon_data);
        g_free (display_data);
        g_free (code);

        return bc;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Validate data for Code 39.                                     */
/*--------------------------------------------------------------------------*/
static gboolean
code39_is_data_valid (const gchar *data)
{
        gchar *p;
        gchar  c;

        if (!data || (*data == '\0'))
        {
                return FALSE;
        }

        for ( p = (gchar *)data; *p != 0; p++ )
        {
                c = g_ascii_toupper (*p);

                if ( strchr(alphabet, c) == NULL )
                {
                        return FALSE;
                }
        }

        return TRUE;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Validate data for Extended Code 39.                            */
/*--------------------------------------------------------------------------*/
static gboolean
code39_ext_is_data_valid (const gchar *data)
{
        gchar *p;

        if (!data || (*data == '\0'))
        {
                return FALSE;
        }

        for ( p = (gchar *)data; *p != 0; p++ )
        {
                if ( (*p < 0) || (*p > 0x7f)  )
                {
                        return FALSE;
                }
        }

        return TRUE;
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Generate string of symbols, representing barcode.              */
/*--------------------------------------------------------------------------*/
static gchar *
code39_encode (const gchar *data,
               gboolean     checksum_flag)
{
        gchar         *p, c;
        gint           c_value, sum;
        GString       *code;


        /* Left frame symbol */
        code = g_string_new( frame_symbol );
        code = g_string_append( code, "i" );

        sum = 0;
        for ( p=(gchar *)data; *p != 0; p++ )
        {
                c = g_ascii_toupper( *p );
                c_value = strchr(alphabet, c) - alphabet;
                code = g_string_append (code, symbols[c_value]);
                code = g_string_append (code, "i");

                sum += c_value;
        }

        if ( checksum_flag )
        {
                code = g_string_append (code, symbols[sum % 43]);
                code = g_string_append (code, "i");
        }

        /* Right frame bar */
        code = g_string_append (code, frame_symbol);

        return g_string_free (code, FALSE);
}


/*--------------------------------------------------------------------------*/
/* PRIVATE.  Vectorize encoded barcode.                                     */
/*--------------------------------------------------------------------------*/
static lglBarcode *
code39_vectorize (const gchar   *code,
                  gdouble        w,
                  gdouble        h,
                  gboolean       text_flag,
                  gboolean       checksum_flag,
                  const gchar   *data,
                  const gchar   *string)
{
        gint         n_chars;
        gdouble      min_l;
        gdouble      scale;
        gdouble      width, height;
        gdouble      x_quiet;
        lglBarcode  *bc;
        gchar       *p;
        gdouble      x1;
        gchar       *string_plus_stars;

        /* determine width and establish horizontal scale */
        n_chars = strlen (data);
        if (!checksum_flag)
        {
                min_l = (n_chars + 2)*(3*N + 6)*MIN_X + (n_chars + 1)*MIN_I;
        }
        else
        {
                min_l = (n_chars + 3)*(3*N + 6)*MIN_X + (n_chars + 2)*MIN_I;
        }
        
        if ( w == 0 )
        {
                scale = 1.0;
        }
        else
        {
                scale = w / (min_l + 2*MIN_QUIET);

                if ( scale < 1.0 )
                {
                        scale = 1.0;
                }
        }
        width = min_l * scale;

        /* determine height of barcode */
        height = text_flag ? h - TEXT_AREA_HEIGHT : h;
        height = MAX (height, MAX(0.15*width, MIN_HEIGHT));

        /* determine horizontal quiet zone */
        x_quiet = MAX ((10 * scale * MIN_X), MIN_QUIET);


        bc = lgl_barcode_new ();

        /* Now traverse the code string and create a list of rectangles */
        x1 = x_quiet;
        for ( p = (gchar *)code; *p != 0; p++ )
        {

                switch ( *p )
                {

                case 'i':
                        /* Inter-character gap */
                        x1 += scale * MIN_I;
                        break;

                case 'N':
                        /* Narrow bar */
                        lgl_barcode_add_box (bc, x1, 0.0, (scale * MIN_X - INK_BLEED), height);
                        x1 += scale * MIN_X;
                        break;

                case 'W':
                        /* Wide bar */
                        lgl_barcode_add_box (bc, x1, 0.0, (scale * N * MIN_X - INK_BLEED), height);
                        x1 += scale * N * MIN_X;
                        break;

                case 'n':
                        /* Narrow space */
                        x1 += scale * MIN_X;
                        break;

                case 'w':
                        /* Wide space */
                        x1 += scale * N * MIN_X;
                        break;

                default:
                        g_message( "Invalid Code39 symbol, should not happen" );
                        break;
                }
        }

        if ( text_flag )
        {
                string_plus_stars = g_strdup_printf ("*%s*", string);
                lgl_barcode_add_string (bc,
                                        x_quiet + width/2, height + (TEXT_AREA_HEIGHT-TEXT_SIZE)/2,
                                        TEXT_SIZE, string_plus_stars, strlen (string_plus_stars));
                g_free (string_plus_stars);
        }

        bc->width  = width + 2*x_quiet;
        bc->height = text_flag ? height + TEXT_AREA_HEIGHT : height;

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
