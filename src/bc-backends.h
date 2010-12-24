/*
 *  bc-backends.h
 *  Copyright (C) 2001-2010  Jim Evins <evins@snaught.com>.
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

#ifndef __BC_BACKENDS_H__
#define __BC_BACKENDS_H__

#include <glib.h>
#include <libglbarcode.h>

G_BEGIN_DECLS


GList           *gl_barcode_backends_get_backend_list     (void);
void             gl_barcode_backends_free_backend_list    (GList          *backend_list);

const gchar     *gl_barcode_backends_backend_id_to_name   (const gchar    *backend_id);
const gchar     *gl_barcode_backends_backend_name_to_id   (const gchar    *backend_name);

gboolean         gl_barcode_backends_is_backend_id_valid  (const gchar    *backend_id);
const gchar     *gl_barcode_backends_guess_backend_id     (const gchar    *id);

GList           *gl_barcode_backends_get_styles_list      (const gchar    *backend_id);
void             gl_barcode_backends_free_styles_list     (GList          *styles_list);

const gchar     *gl_barcode_backends_style_id_to_name     (const gchar    *backend_id,
                                                           const gchar    *id);
const gchar     *gl_barcode_backends_style_name_to_id     (const gchar    *backend_id,
                                                           const gchar    *name);

gchar           *gl_barcode_backends_style_default_digits (const gchar    *backend_id,
                                                           const gchar    *id,
                                                           guint           n);

gboolean         gl_barcode_backends_style_can_text       (const gchar    *backend_id,
                                                           const gchar    *id);
gboolean         gl_barcode_backends_style_text_optional  (const gchar    *backend_id,
                                                           const gchar    *id);

gboolean         gl_barcode_backends_style_can_csum       (const gchar    *backend_id,
                                                           const gchar    *id);
gboolean         gl_barcode_backends_style_csum_optional  (const gchar    *backend_id,
                                                           const gchar    *id);

gboolean         gl_barcode_backends_style_can_freeform   (const gchar    *backend_id,
                                                           const gchar    *id);
guint            gl_barcode_backends_style_get_prefered_n (const gchar    *backend_id,
                                                           const gchar    *id);

lglBarcode      *gl_barcode_backends_new_barcode          (const gchar    *backend_id,
                                                           const gchar    *id,
                                                           gboolean        text_flag,
                                                           gboolean        checksum_flag,
                                                           gdouble         w,
                                                           gdouble         h,
                                                           const gchar    *digits);




G_END_DECLS

#endif /* __BC_BACKENDS_H__ */



/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
