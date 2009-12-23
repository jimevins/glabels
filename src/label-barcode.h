/*
 *  label-barcode.h
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

#ifndef __LABEL_BARCODE_H__
#define __LABEL_BARCODE_H__

#include "bc.h"
#include "text-node.h"
#include "label-object.h"

G_BEGIN_DECLS


#define GL_TYPE_LABEL_BARCODE            (gl_label_barcode_get_type ())
#define GL_LABEL_BARCODE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_LABEL_BARCODE, glLabelBarcode))
#define GL_LABEL_BARCODE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_LABEL_BARCODE, glLabelBarcodeClass))
#define GL_IS_LABEL_BARCODE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_LABEL_BARCODE))
#define GL_IS_LABEL_BARCODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_LABEL_BARCODE))

typedef struct _glLabelBarcode          glLabelBarcode;
typedef struct _glLabelBarcodeClass     glLabelBarcodeClass;

typedef struct _glLabelBarcodePrivate   glLabelBarcodePrivate;

struct _glLabelBarcode {
	glLabelObject            object;

	glLabelBarcodePrivate   *priv;
};

struct _glLabelBarcodeClass {
	glLabelObjectClass       parent_class;
};


GType           gl_label_barcode_get_type  (void) G_GNUC_CONST;

GObject        *gl_label_barcode_new       (glLabel        *label,
                                            gboolean        checkpoint);

void            gl_label_barcode_set_data  (glLabelBarcode *lbc,
					    glTextNode     *text_node,
                                            gboolean        checkpoint);

void            gl_label_barcode_set_props (glLabelBarcode *lbc,
					    gchar          *id,
					    gboolean        text_flag,
					    gboolean        checksum_flag,
					    guint           format_digits,
                                            gboolean        checkpoint);

glTextNode     *gl_label_barcode_get_data  (glLabelBarcode *lbc);

void            gl_label_barcode_get_props (glLabelBarcode *lbc,
					    gchar         **id,
					    gboolean       *text_flag,
					    gboolean       *checksum_flag,
					    guint          *format_digits);

G_END_DECLS

#endif /* __LABEL_BARCODE_H__ */




/*
 * Local Variables:       -- emacs
 * mode: C                -- emacs
 * c-basic-offset: 8      -- emacs
 * tab-width: 8           -- emacs
 * indent-tabs-mode: nil  -- emacs
 * End:                   -- emacs
 */
