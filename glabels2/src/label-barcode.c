/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  label_barcode.c:  GLabels label text object
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

#include <glib.h>
#include <libgnomeprint/gnome-glyphlist.h>

#include "label-barcode.h"

#include "pixmaps/checkerboard.xpm"

#include "debug.h"

/*========================================================*/
/* Private macros and constants.                          */
/*========================================================*/

/*========================================================*/
/* Private types.                                         */
/*========================================================*/

struct _glLabelBarcodePrivate {
	glTextNode     *text_node;
	glBarcodeStyle  style;
	guint           color;
	gboolean        text_flag;
	gboolean        checksum_flag;
};

/*========================================================*/
/* Private globals.                                       */
/*========================================================*/

static glLabelObjectClass *parent_class = NULL;

static guint instance = 0;

/*========================================================*/
/* Private function prototypes.                           */
/*========================================================*/

static void  gl_label_barcode_class_init    (glLabelBarcodeClass *klass);
static void  gl_label_barcode_instance_init (glLabelBarcode      *lbc);
static void  gl_label_barcode_finalize      (GObject             *object);

static void  copy                           (glLabelObject       *dst_object,
					     glLabelObject       *src_object);

static void  get_size                       (glLabelObject       *object,
					     gdouble             *w,
					     gdouble             *h);

static void  set_line_color                 (glLabelObject       *object,
					     guint                line_color);

static guint get_line_color                 (glLabelObject       *object);



/*****************************************************************************/
/* Boilerplate object stuff.                                                 */
/*****************************************************************************/
GType
gl_label_barcode_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (glLabelBarcodeClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_label_barcode_class_init,
			NULL,
			NULL,
			sizeof (glLabelBarcode),
			0,
			(GInstanceInitFunc) gl_label_barcode_instance_init,
		};

		type = g_type_register_static (GL_TYPE_LABEL_OBJECT,
					       "glLabelBarcode", &info, 0);
	}

	return type;
}

static void
gl_label_barcode_class_init (glLabelBarcodeClass *klass)
{
	GObjectClass       *object_class       = (GObjectClass *) klass;
	glLabelObjectClass *label_object_class = (glLabelObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	label_object_class->copy           = copy;
	label_object_class->get_size       = get_size;
	label_object_class->set_line_color = set_line_color;
	label_object_class->get_line_color = get_line_color;

	object_class->finalize = gl_label_barcode_finalize;
}

static void
gl_label_barcode_instance_init (glLabelBarcode *lbc)
{
	lbc->private = g_new0 (glLabelBarcodePrivate, 1);
}

static void
gl_label_barcode_finalize (GObject *object)
{
	glLabelBarcode *lbc;

	g_return_if_fail (object && GL_IS_LABEL_BARCODE (object));

	lbc = GL_LABEL_BARCODE (object);

	gl_text_node_free (&lbc->private->text_node);
	g_free (lbc->private);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/*****************************************************************************/
/* NEW label "text" object.                                               */
/*****************************************************************************/
GObject *
gl_label_barcode_new (glLabel *label)
{
	glLabelBarcode *lbc;

	lbc = g_object_new (gl_label_barcode_get_type(), NULL);

	gl_label_object_set_parent (GL_LABEL_OBJECT(lbc), label);

	return G_OBJECT (lbc);
}

/*****************************************************************************/
/* Copy object contents.                                                     */
/*****************************************************************************/
static void
copy (glLabelObject *dst_object,
      glLabelObject *src_object)
{
	glLabelBarcode      *lbc     = (glLabelBarcode *)src_object;
	glLabelBarcode      *new_lbc = (glLabelBarcode *)dst_object;
	glTextNode          *text_node;
	glBarcodeStyle       style;
	gboolean             text_flag;
	gboolean             checksum_flag;
	guint                color;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (lbc && GL_IS_LABEL_BARCODE (lbc));
	g_return_if_fail (new_lbc && GL_IS_LABEL_BARCODE (new_lbc));

	text_node = gl_label_barcode_get_data (lbc);
	gl_label_barcode_get_props (lbc, &style, &text_flag, &checksum_flag);
	color = get_line_color (src_object);

	gl_label_barcode_set_data (new_lbc, text_node);
	gl_label_barcode_set_props (new_lbc,style, text_flag, checksum_flag);
	set_line_color (dst_object, color);

	gl_text_node_free (&text_node);

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Set object params.                                                        */
/*****************************************************************************/
void
gl_label_barcode_set_data (glLabelBarcode *lbc,
			   glTextNode     *text_node)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (lbc && GL_IS_LABEL_BARCODE (lbc));

	gl_text_node_free (&lbc->private->text_node);
	lbc->private->text_node = gl_text_node_dup (text_node);

	gl_label_object_emit_changed (GL_LABEL_OBJECT(lbc));

	gl_debug (DEBUG_LABEL, "END");
}

void
gl_label_barcode_set_props (glLabelBarcode *lbc,
			    glBarcodeStyle  style,
			    gboolean        text_flag,
			    gboolean        checksum_flag)
{
	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (lbc && GL_IS_LABEL_BARCODE (lbc));

	lbc->private->style            = style;
	lbc->private->text_flag        = text_flag;
	lbc->private->checksum_flag    = checksum_flag;

	gl_label_object_emit_changed (GL_LABEL_OBJECT(lbc));

	gl_debug (DEBUG_LABEL, "END");
}


/*****************************************************************************/
/* Get object params.                                                        */
/*****************************************************************************/
glTextNode *
gl_label_barcode_get_data (glLabelBarcode *lbc)
{
	g_return_val_if_fail (lbc && GL_IS_LABEL_BARCODE (lbc), NULL);

	return gl_text_node_dup (lbc->private->text_node);
}

void
gl_label_barcode_get_props (glLabelBarcode *lbc,
			    glBarcodeStyle *style,
			    gboolean       *text_flag,
			    gboolean       *checksum_flag)
{
	g_return_if_fail (lbc && GL_IS_LABEL_BARCODE (lbc));

	*style            = lbc->private->style;
	*text_flag        = lbc->private->text_flag;
	*checksum_flag    = lbc->private->checksum_flag;
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Get object size method.                                         */
/*---------------------------------------------------------------------------*/
static void
get_size (glLabelObject *object,
	  gdouble       *w,
	  gdouble       *h)
{
	glLabelBarcode      *lbc = (glLabelBarcode *)object;
	gchar               *data;
	gdouble              w_parent, h_parent;
	glBarcode           *gbc;

	gl_debug (DEBUG_LABEL, "START");

	g_return_if_fail (lbc && GL_IS_LABEL_BARCODE (lbc));

	(* parent_class->get_size) (object, &w_parent, &h_parent);


	if (lbc->private->text_node->field_flag) {
		data = gl_barcode_default_digits (lbc->private->style);
	} else {
		data = gl_text_node_expand (lbc->private->text_node, NULL);
	}

	gbc = gl_barcode_new (lbc->private->style,
			      lbc->private->text_flag,
			      lbc->private->checksum_flag,
			      w_parent,
			      h_parent,
			      data);

	if ( gbc == NULL ) {
		/* Try again with default digits. */
		data = gl_barcode_default_digits (lbc->private->style);
		gbc = gl_barcode_new (lbc->private->style,
				      lbc->private->text_flag,
				      lbc->private->checksum_flag,
				      w_parent,
				      h_parent,
				      data);
	}

	*w = gbc->width;
	*h = gbc->height;

	gl_barcode_free (&gbc);
	g_free (data);

	gl_debug (DEBUG_LABEL, "END");
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Set line color method.                                          */
/*---------------------------------------------------------------------------*/
static void
set_line_color (glLabelObject *object,
		guint          line_color)
{
	glLabelBarcode *lbarcode = (glLabelBarcode *)object;

	g_return_if_fail (lbarcode && GL_IS_LABEL_BARCODE (lbarcode));

	if ( lbarcode->private->color != line_color ) {
		lbarcode->private->color = line_color;
		gl_label_object_emit_changed (GL_LABEL_OBJECT(lbarcode));
	}
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Get line color method.                                          */
/*---------------------------------------------------------------------------*/
static guint
get_line_color (glLabelObject *object)
{
	glLabelBarcode *lbarcode = (glLabelBarcode *)object;

	g_return_if_fail (lbarcode && GL_IS_LABEL_BARCODE (lbarcode));

	return lbarcode->private->color;
}

