/*
 *  bc-backends.c
 *  Copyright (C) 2001-2009  Jim Evins <evins@snaught.com>.
 *
 *  This file is part of gLabels.
 *
 *  gLabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gLabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gLabels.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include "bc-backends.h"

#include <glib.h>
#include <glib/gi18n.h>

#include "bc-builtin.h"
#include "bc-gnubarcode.h"
#include "bc-zint.h"
#include "bc-iec16022.h"
#include "bc-iec18004.h"

#include "debug.h"


/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/


/*========================================================*/
/* Private types.                                         */
/*========================================================*/

typedef lglBarcode *(*glBarcodeNewFunc) (const gchar    *id,
                                         gboolean        text_flag,
                                         gboolean        checksum_flag,
                                         gdouble         w,
                                         gdouble         h,
                                         const gchar    *digits);


typedef struct {
        gchar            *id;
        gchar            *name;
} Backend;


typedef struct {
        gchar            *backend_id;
        gchar            *id;
        gchar            *name;
        glBarcodeNewFunc  new_barcode;
        gboolean          can_text;
        gboolean          text_optional;
        gboolean          can_checksum;
        gboolean          checksum_optional;
        gchar            *default_digits;
        gboolean          can_freeform;
        guint             prefered_n;
} Style;


/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static const Backend backends[] = {

        { "built-in",    N_("Built-in") },
#ifdef HAVE_LIBBARCODE
        { "gnu-barcode", "GNU Barcode" },
#endif
#ifdef HAVE_LIBZINT
        { "zint",        "Zint" },
#endif
#ifdef HAVE_LIBIEC16022
        { "libiec16022", "IEC16022" },
#endif
#ifdef HAVE_LIBQRENCODE
        { "libqrencode", "QREncode" },
#endif

        { NULL, NULL }
};


static const Style styles[] = {

        { "built-in", "POSTNET", N_("POSTNET (any)"), gl_barcode_builtin_new,
          FALSE, FALSE, TRUE, FALSE, "12345-6789-12", FALSE, 11},

        { "built-in", "POSTNET-5", N_("POSTNET-5 (ZIP only)"), gl_barcode_builtin_new,
          FALSE, FALSE, TRUE, FALSE, "12345", FALSE, 5},

        { "built-in", "POSTNET-9", N_("POSTNET-9 (ZIP+4)"), gl_barcode_builtin_new,
          FALSE, FALSE, TRUE, FALSE, "12345-6789", FALSE, 9},

        { "built-in", "POSTNET-11", N_("POSTNET-11 (DPBC)"), gl_barcode_builtin_new,
          FALSE, FALSE, TRUE, FALSE, "12345-6789-12", FALSE, 11},

        { "built-in", "CEPNET", N_("CEPNET"), gl_barcode_builtin_new,
          FALSE, FALSE, TRUE, FALSE, "12345-678", FALSE, 8},

        { "built-in", "ONECODE", N_("One Code"), gl_barcode_builtin_new,
          FALSE, FALSE, TRUE, FALSE, "12345678901234567890", FALSE, 20},

        { "built-in", "Code39", N_("Code 39"), gl_barcode_builtin_new,
          TRUE, TRUE, TRUE, TRUE, "1234567890", TRUE, 10},

        { "built-in", "Code39Ext", N_("Code 39 Extended"), gl_barcode_builtin_new,
          TRUE, TRUE, TRUE, TRUE, "1234567890", TRUE, 10},

#ifdef HAVE_LIBBARCODE

        { "gnu-barcode", "EAN", N_("EAN (any)"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, FALSE, "000000000000 00000", FALSE, 17},

        { "gnu-barcode", "EAN-8", N_("EAN-8"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, FALSE, "0000000", FALSE, 7},

        { "gnu-barcode", "EAN-8+2", N_("EAN-8 +2"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, FALSE, "0000000 00", FALSE, 9},

        { "gnu-barcode", "EAN-8+5", N_("EAN-8 +5"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, FALSE, "0000000 00000", FALSE, 12},

        { "gnu-barcode", "EAN-13", N_("EAN-13"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, FALSE, "000000000000", FALSE, 12},

        { "gnu-barcode", "EAN-13+2", N_("EAN-13 +2"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, FALSE, "000000000000 00", FALSE, 14},

        { "gnu-barcode", "EAN-13+5", N_("EAN-13 +5"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, FALSE, "000000000000 00000", FALSE, 17},

        { "gnu-barcode", "UPC", N_("UPC (UPC-A or UPC-E)"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, FALSE, "00000000000 00000", FALSE, 16},

        { "gnu-barcode", "UPC-A", N_("UPC-A"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, FALSE, "00000000000", FALSE, 11},

        { "gnu-barcode", "UPC-A+2", N_("UPC-A +2"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, FALSE, "00000000000 00", FALSE, 13},

        { "gnu-barcode", "UPC-A+5", N_("UPC-A +5"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, FALSE, "00000000000 00000", FALSE, 16},

        { "gnu-barcode", "UPC-E", N_("UPC-E"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, FALSE, "000000", FALSE, 6},

        { "gnu-barcode", "UPC-E+2", N_("UPC-E +2"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, FALSE, "000000 00", FALSE, 8},

        { "gnu-barcode", "UPC-E+5", N_("UPC-E +5"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, FALSE, "000000 00000", FALSE, 11},

        { "gnu-barcode", "ISBN", N_("ISBN"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, TRUE, "0-00000-000-0", FALSE, 10},

        { "gnu-barcode", "ISBN+5", N_("ISBN +5"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, TRUE, "0-00000-000-0 00000", FALSE, 15},

        { "gnu-barcode", "Code39", N_("Code 39"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, TRUE, "0000000000", TRUE, 10},

        { "gnu-barcode", "Code128", N_("Code 128"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, TRUE, "0000000000", TRUE, 10},

        { "gnu-barcode", "Code128C", N_("Code 128C"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "gnu-barcode", "Code128B", N_("Code 128B"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, TRUE, "0000000000", TRUE, 10},

        { "gnu-barcode", "I25", N_("Interleaved 2 of 5"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, TRUE, "0000000000", TRUE, 10},

        { "gnu-barcode", "CBR", N_("Codabar"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, TRUE, "0000000000", TRUE, 10},

        { "gnu-barcode", "MSI", N_("MSI"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, TRUE, "0000000000", TRUE, 10},

        { "gnu-barcode", "PLS", N_("Plessey"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, TRUE, "0000000000", TRUE, 10},

        { "gnu-barcode", "Code93", N_("Code 93"), gl_barcode_gnubarcode_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},

#endif /* HAVE_LIBBARCODE */

#ifdef HAVE_LIBZINT

        { "zint", "AUSP", N_("Australia Post Standard"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "12345678901234567890123", TRUE, 23},

        { "zint", "AUSRP", N_("Australia Post Reply Paid"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "12345678", TRUE, 8},

        { "zint", "AUSRT", N_("Australia Post Route Code"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "12345678", TRUE, 8},

        { "zint", "AUSRD", N_("Australia Post Redirect"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "12345678", TRUE, 8},

        { "zint", "AZTEC", N_("Aztec Code"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "1234567890", TRUE, 10},
          
        { "zint", "AZRUN", N_("Aztec Rune"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "255", TRUE, 3},

        { "zint", "CBR", N_("Codabar"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "ABCDABCDAB", TRUE, 10},

        { "zint", "Code1", N_("Code One"),  gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "Code11", N_("Code 11"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},
          
        { "zint", "C16K", N_("Code 16K"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "0000000000", TRUE, 10},
          
        { "zint", "C25M", N_("Code 2 of 5 Matrix"),  gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},
          
        { "zint", "C25I", N_("Code 2 of 5 IATA"),  gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},
          
        { "zint", "C25DL", N_("Code 2 of 5 Data Logic"),  gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "Code32", N_("Code 32 (Italian Pharmacode)"),  gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "12345678", TRUE, 8},

        { "zint", "Code39", N_("Code 39"), gl_barcode_zint_new,
          TRUE, TRUE, FALSE, FALSE, "0000000000", TRUE, 10},
          
        { "zint", "Code39E", N_("Code 39 Extended"),  gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "Code49", N_("Code 49"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "Code93", N_("Code 93"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "Code128", N_("Code 128"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},
          
        { "zint", "Code128B", N_("Code 128 (Mode C suppression)"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},
          
        { "zint", "DAFT", N_("DAFT Code"), gl_barcode_zint_new,
          FALSE, FALSE, FALSE, FALSE, "DAFTDAFTDAFTDAFT", TRUE, 16},

        { "zint", "DMTX", N_("Data Matrix"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "DPL", N_("Deutsche Post Leitcode"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "1234567890123", TRUE, 13},
          
        { "zint", "DPI", N_("Deutsche Post Identcode"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "12345678901", TRUE, 11},
          
        { "zint", "KIX", N_("Dutch Post KIX Code"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "EAN", N_("EAN"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "1234567890123", FALSE, 13},

        { "zint", "GMTX", N_("Grid Matrix"),  gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "GS1-128", N_("GS1-128"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "[01]12345678901234", FALSE, 18},

        { "zint", "RSS14", N_("GS1 DataBar-14"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "1234567890123", TRUE, 13},
          
        { "zint", "RSSLTD", "GS1 DataBar-14 Limited",  gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "1234567890123", TRUE, 13},
          
        { "zint", "RSSEXP", "GS1 DataBar Extended",  gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "[01]12345678901234", FALSE, 18},
          
        { "zint", "RSSS", N_("GS1 DataBar-14 Stacked"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "RSSSO", N_("GS1 DataBar-14 Stacked Omni."), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "RSSSE", N_("GS1 DataBar Extended Stacked"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "[01]12345678901234", FALSE, 18},

        { "zint", "HIBC128", N_("HIBC Code 128"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "HIBC39", N_("HIBC Code 39"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "HIBCDM", N_("HIBC Data Matrix"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "HIBCQR", N_("HIBC QR Code"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "HIBCPDF", N_("HIBC PDF417"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "HIBCMPDF", N_("HIBC Micro PDF417"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "HIBCAZ", N_("HIBC Aztec Code"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "I25", N_("Interleaved 2 of 5"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "ISBN", N_("ISBN"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "123456789", FALSE, 9},

        { "zint", "ITF14", N_("ITF-14"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "JAPAN", N_("Japanese Postal"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "KOREA", N_("Korean Postal"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "123456", FALSE, 6},

        { "zint", "LOGM", N_("LOGMARS"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "MAXI", N_("Maxicode"), gl_barcode_zint_new,
          FALSE, FALSE, FALSE, FALSE, "0000000000", TRUE, 10},

        { "zint", "MPDF", N_("Micro PDF417"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "MQR", N_("Micro QR Code"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "MSI", N_("MSI Plessey"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "NVE", N_("NVE-18"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "12345678901234567", FALSE, 17},

        { "zint", "PDF", N_("PDF417"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "PDFT", N_("PDF417 Truncated"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "PLAN", N_("PLANET"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "POSTNET", N_("PostNet"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "PHARMA", N_("Pharmacode"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "123456", FALSE, 6},

        { "zint", "PHARMA2", N_("Pharmacode 2-track"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "12345678", FALSE, 8},

        { "zint", "PZN", N_("Pharmazentral Nummer (PZN)"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "123456", FALSE, 6},

        { "zint", "QR", N_("QR Code"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "RM4", N_("Royal Mail 4-State"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "TELE", N_("Telepen"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "TELEX", N_("Telepen Numeric"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},

        { "zint", "UPC-A", N_("UPC-A"),  gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "12345678901", FALSE, 11},
          
        { "zint", "UPC-E", N_("UPC-E"),  gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "1234567", FALSE, 7},
          
        { "zint", "USPS", N_("USPS One Code"), gl_barcode_zint_new,
          FALSE, FALSE, TRUE, FALSE, "12345678901234567890", TRUE, 20},

        { "zint", "PLS", N_("UK Plessey"), gl_barcode_zint_new,
          TRUE, TRUE, TRUE, FALSE, "0000000000", TRUE, 10},

#endif /* HAVE_LIBZINT */

#ifdef HAVE_LIBIEC16022

        { "libiec16022", "IEC16022", N_("IEC16022 (DataMatrix)"), gl_barcode_iec16022_new,
          FALSE, FALSE, TRUE, FALSE, "12345678", TRUE, 8},

#endif /* HAVE_LIBIEC16022 */

#ifdef HAVE_LIBQRENCODE

        { "libqrencode", "IEC18004", N_("IEC18004 (QRCode)"), gl_barcode_iec18004_new,
          FALSE, FALSE, TRUE, FALSE, "12345678", TRUE, 8},

#endif /* HAVE_LIBQRENCODE */

        { NULL, NULL, NULL, NULL, FALSE, FALSE, FALSE, FALSE, NULL, FALSE, 0}

};


/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static gint backend_id_to_index   (const gchar *id);
static gint backend_name_to_index (const gchar *name);
static gint style_id_to_index     (const gchar *backend_id,
                                   const gchar *id);
static gint style_name_to_index   (const gchar *backend_id,
                                   const gchar *name);

/*---------------------------------------------------------------------------*/
/* Convert backend id to index into backends table.                          */
/*---------------------------------------------------------------------------*/
static gint
backend_id_to_index (const gchar *id)
{
        gint i;

        if (id == NULL)
        {
                return 0; /* NULL request default. I.e., the first element. */
        }

        for (i=0; styles[i].id != NULL; i++)
        {
                if (g_ascii_strcasecmp (id, backends[i].id) == 0)
                {
                        return i;
                }
        }

        g_message( "Unknown barcode id \"%s\"", id );
        return 0;
}


/*---------------------------------------------------------------------------*/
/* Convert backend name to index into backends table.                        */
/*---------------------------------------------------------------------------*/
static gint
backend_name_to_index (const gchar *name)
{
        gint i;

        if (name == NULL)
        {
                return 0; /* NULL request default. I.e., the first element. */
        }

        for (i=0; styles[i].id != NULL; i++)
        {
                if (strcmp (name, gettext (backends[i].name)) == 0)
                {
                        return i;
                }
        }

        g_message( "Unknown barcode name \"%s\"", name );
        return 0;
}


/*---------------------------------------------------------------------------*/
/* Convert style id to index into styles table.                              */
/*---------------------------------------------------------------------------*/
static gint
style_id_to_index (const gchar *backend_id,
                   const gchar *id)
{
        gint i;

        if (backend_id == NULL)
        {
                return 0; /* NULL request default. I.e., the first element. */
        }

        if (id == NULL)
        {
                /* Search for first element with given backend_id. */
                for (i=0; styles[i].id != NULL; i++)
                {
                        if (g_ascii_strcasecmp (backend_id, styles[i].backend_id) == 0)
                        {
                                return i;
                        }
                }
                g_message( "Unknown barcode backend id \"%s\"", backend_id );
                return 0;
        }

        for (i=0; styles[i].id != NULL; i++)
        {
                if ((g_ascii_strcasecmp (backend_id, styles[i].backend_id) == 0) &&
                    (g_ascii_strcasecmp (id, styles[i].id) == 0))
                {
                        return i;
                }
        }

        g_message( "Unknown barcode id \"%s\"", id );
        return 0;
}


/*---------------------------------------------------------------------------*/
/* Convert style name to index into styles table.                            */
/*---------------------------------------------------------------------------*/
static gint
style_name_to_index (const gchar *backend_id,
                     const gchar *name)
{
        gint i;

        if (backend_id == NULL)
        {
                return 0; /* NULL request default. I.e., the first element. */
        }

        if (name == NULL)
        {
                /* Search for first element with given backend_id. */
                for (i=0; styles[i].id != NULL; i++)
                {
                        if (g_ascii_strcasecmp (backend_id, styles[i].backend_id) == 0)
                        {
                                return i;
                        }
                }
                g_message( "Unknown barcode backend id \"%s\"", backend_id );
                return 0;
        }

        for (i=0; styles[i].id != NULL; i++)
        {
                if ((g_ascii_strcasecmp (backend_id, styles[i].backend_id) == 0) &&
                    (strcmp (name, gettext (styles[i].name)) == 0))
                {
                        return i;
                }
        }

        g_message( "Unknown barcode name \"%s\"", name );
        return 0;
}


/*****************************************************************************/
/* Get a list of names for configured backends.                              */
/*****************************************************************************/
GList *
gl_barcode_backends_get_backend_list (void)
{
        gint   i;
        GList *list = NULL;

        for (i=0; backends[i].id != NULL; i++)
        {
                list = g_list_prepend (list, g_strdup (gettext (backends[i].name)));
        }

        return g_list_reverse (list);
}


/*****************************************************************************/
/* Free up a previously allocated list of backend names.                     */
/*****************************************************************************/
void
gl_barcode_backends_free_backend_list (GList *backend_list)
{
        GList *p;

        for (p=backend_list; p != NULL; p=p->next)
        {
                g_free (p->data);
                p->data = NULL;
        }

        g_list_free (backend_list);
}


/*****************************************************************************/
/* Convert backend id to name.                                               */
/*****************************************************************************/
const gchar *
gl_barcode_backends_backend_id_to_name (const gchar *backend_id)
{
        return gettext (backends[backend_id_to_index (backend_id)].name);
}


/*****************************************************************************/
/* Convert backend name to id.                                               */
/*****************************************************************************/
const gchar *
gl_barcode_backends_backend_name_to_id (const gchar *backend_name)
{
        return backends[backend_name_to_index (backend_name)].id;
}


/*****************************************************************************/
/* Test if backend id is valid.                                              */
/*****************************************************************************/
gboolean
gl_barcode_backends_is_backend_id_valid (const gchar    *backend_id)
{
        gint i;

        if (backend_id == NULL)
        {
                return FALSE;
        }

        for (i=0; backends[i].id != NULL; i++)
        {
                if (g_ascii_strcasecmp (backend_id, backends[i].id) == 0)
                {
                        return TRUE;
                }
        }

        return FALSE;
}


/*****************************************************************************/
/* Guess backend id from style id (for backwards compatability).             */
/*****************************************************************************/
const gchar *
gl_barcode_backends_guess_backend_id (const gchar *id)
{
        gint i;

        if (id == NULL)
        {
                return styles[0].backend_id;
        }

        for (i=0; styles[i].id != NULL; i++)
        {
                if (g_ascii_strcasecmp (id, styles[i].id) == 0)
                {
                        return styles[i].backend_id;
                }
        }

        g_message( "Unknown barcode id \"%s\"", id );
        return styles[0].backend_id;
}


/*****************************************************************************/
/* Get a list of names for valid barcode styles.                             */
/*****************************************************************************/
GList *
gl_barcode_backends_get_styles_list  (const gchar *backend_id)
{
        gint   i;
        GList *list = NULL;

        for (i=0; styles[i].id != NULL; i++)
        {
                if ( g_ascii_strcasecmp (styles[i].backend_id, backend_id) == 0 )
                {
                        list = g_list_prepend (list, g_strdup (gettext (styles[i].name)));
                }
        }

        return g_list_reverse (list);
}


/*****************************************************************************/
/* Free up a previously allocated list of style names.                       */
/*****************************************************************************/
void
gl_barcode_backends_free_styles_list (GList *styles_list)
{
        GList *p;

        for (p=styles_list; p != NULL; p=p->next)
        {
                g_free (p->data);
                p->data = NULL;
        }

        g_list_free (styles_list);
}


/*****************************************************************************/
/* Convert style to text.                                                    */
/*****************************************************************************/
const gchar *
gl_barcode_backends_style_id_to_name (const gchar *backend_id,
                                      const gchar *id)
{
        return gettext (styles[style_id_to_index (backend_id, id)].name);
}


/*****************************************************************************/
/* Convert name to style.                                                    */
/*****************************************************************************/
const gchar *
gl_barcode_backends_style_name_to_id (const gchar *backend_id,
                                      const gchar *name)
{
        return styles[style_name_to_index (backend_id, name)].id;
}


/*****************************************************************************/
/* Return an appropriate set of digits for the given barcode style.          */
/*****************************************************************************/
gchar *
gl_barcode_backends_style_default_digits (const gchar *backend_id,
                                          const gchar *id,
                                          guint        n)
{
        int i;

        i = style_id_to_index (backend_id, id);

        if (styles[i].can_freeform)
        {
                return g_strnfill (MAX (n,1), '0');
        }
        else
        {
                return g_strdup (styles[i].default_digits);
        }
}


/*****************************************************************************/
/* Query text capabilities.                                                  */
/*****************************************************************************/
gboolean
gl_barcode_backends_style_can_text (const gchar *backend_id,
                                    const gchar *id)
{
        return styles[style_id_to_index (backend_id, id)].can_text;
}


gboolean
gl_barcode_backends_style_text_optional (const gchar *backend_id,
                                         const gchar *id)
{
        return styles[style_id_to_index (backend_id, id)].text_optional;
}


/*****************************************************************************/
/* Query checksum capabilities.                                              */
/*****************************************************************************/
gboolean
gl_barcode_backends_style_can_csum (const gchar *backend_id,
                                    const gchar *id)
{
        return styles[style_id_to_index (backend_id, id)].can_checksum;
}


gboolean
gl_barcode_backends_style_csum_optional (const gchar *backend_id,
                                         const gchar *id)
{
        return styles[style_id_to_index (backend_id, id)].checksum_optional;
}


/*****************************************************************************/
/* Query if freeform input is allowed.                                       */
/*****************************************************************************/
gboolean
gl_barcode_backends_style_can_freeform (const gchar *backend_id,
                                        const gchar    *id)
{
        return styles[style_id_to_index (backend_id, id)].can_freeform;
}


/*****************************************************************************/
/* Query prefered number of digits of input.                                 */
/*****************************************************************************/
guint
gl_barcode_backends_style_get_prefered_n (const gchar *backend_id,
                                          const gchar *id)
{
        return styles[style_id_to_index (backend_id, id)].prefered_n;
}


/*****************************************************************************/
/* Call appropriate barcode backend to create barcode in intermediate format.*/
/*****************************************************************************/
lglBarcode *
gl_barcode_backends_new_barcode (const gchar    *backend_id,
                                 const gchar    *id,
                                 gboolean        text_flag,
                                 gboolean        checksum_flag,
                                 gdouble         w,
                                 gdouble         h,
                                 const gchar    *digits)
{
        lglBarcode *gbc;
        gint        i;

        g_return_val_if_fail (digits!=NULL, NULL);

        i = style_id_to_index (backend_id, id);

        gbc = styles[i].new_barcode (styles[i].id,
                                     text_flag,
                                     checksum_flag,
                                     w,
                                     h,
                                     digits);

        return gbc;
}



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
