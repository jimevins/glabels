/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label_barcode.h:  GLabels label barcode object
 *
 *  Copyright (C) 2001-2002  Jim Evins <evins@snaught.com>.
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

#ifndef __LABEL_BARCODE_H__
#define __LABEL_BARCODE_H__

#include <gtk/gtkenums.h>
#include <libgnomeprint/gnome-font.h>
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

	glLabelBarcodePrivate   *private;
};

struct _glLabelBarcodeClass {
	glLabelObjectClass       parent_class;
};


GType           gl_label_barcode_get_type  (void);

GObject        *gl_label_barcode_new       (glLabel        *label);

void            gl_label_barcode_set_data  (glLabelBarcode *lbc,
					    glTextNode     *text_node);
void            gl_label_barcode_set_props (glLabelBarcode *lbc,
					    glBarcodeStyle  style,
					    gboolean        text_flag,
					    gboolean        checksum_flag,
					    guint           color);

glTextNode     *gl_label_barcode_get_data  (glLabelBarcode *lbc);
void            gl_label_barcode_get_props (glLabelBarcode *lbc,
					    glBarcodeStyle *style,
					    gboolean       *text_flag,
					    gboolean       *checksum_flag,
					    guint          *color);

G_END_DECLS

#endif /* __LABEL_BARCODE_H__ */
