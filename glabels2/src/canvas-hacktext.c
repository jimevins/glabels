/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-canvas-hacktext.c: Hacktext item type for GnomeCanvas widget
 *                           This is mainly used for gnome-print preview context
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors:
 *    Federico Mena <federico@nuclecu.unam.mx>
 *    Raph Levien <raph@acm.org>
 *    Lauris Kaplinski <lauris@helixcode.com>
 *
 *  Copyright (C) 1998-1999 The Free Software Foundation
 *  Copyright (C) 2000-2002 Ximian Inc.
 *
 *
 *  Modified by Jim Evins <evins@snaught.com> for gLabels.
 */

#include <config.h>

#include <math.h>
#include <string.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-rfont.h>
#include <libgnomeprint/gnome-pgl.h>
#include "canvas-hacktext.h"

enum {
	ARG_0,
	ARG_TEXT,
	ARG_GLYPHLIST,
	ARG_FILL_COLOR,
	ARG_FILL_COLOR_RGBA,
	ARG_FONT,
	ARG_X,
	ARG_Y
};


static void gl_canvas_hacktext_class_init    (glCanvasHacktextClass *class);
static void gl_canvas_hacktext_init          (glCanvasHacktext      *hacktext);
static void gl_canvas_hacktext_destroy       (GtkObject             *object);
static void gl_canvas_hacktext_set_arg       (GObject               *object,
					      guint                  arg_id,
					      const GValue          *value,
					      GParamSpec            *pspec);
static void gl_canvas_hacktext_get_arg       (GObject               *object,
					      guint                  arg_id,
					      GValue                *value,
					      GParamSpec            *pspec);

static void   gl_canvas_hacktext_update      (GnomeCanvasItem *item, double *affine, ArtSVP *clip_path, int flags);
static void   gl_canvas_hacktext_realize     (GnomeCanvasItem *item);
static void   gl_canvas_hacktext_unrealize   (GnomeCanvasItem *item);
static double gl_canvas_hacktext_point       (GnomeCanvasItem *item, double x, double y,
						int cx, int cy, GnomeCanvasItem **actual_item);
static void   gl_canvas_hacktext_bounds      (GnomeCanvasItem *item, double *x1, double *y1, double *x2, double *y2);
static void   gl_canvas_hacktext_render      (GnomeCanvasItem *item, GnomeCanvasBuf *buf);
static void   gl_canvas_hacktext_req_repaint (glCanvasHacktext *hacktext, ArtIRect *bbox);


static GnomeCanvasItemClass *parent_class;

struct _glCanvasHacktextPriv {
	GnomeFont * font;

	GnomeGlyphList * glyphlist;
	GnomePosGlyphList * pgl;
	double affine[6]; /* the text to world transform (NB! mirrored Y) */
};

GType
gl_canvas_hacktext_get_type (void)
{
	static GType hacktext_type = 0;

	if (!hacktext_type) {
		static const GTypeInfo hacktext_info = {
			sizeof (glCanvasHacktextClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_canvas_hacktext_class_init,
			NULL,
			NULL,
			sizeof (glCanvasHacktext),
			0,
			(GInstanceInitFunc) gl_canvas_hacktext_init,
			NULL
		};

		hacktext_type = g_type_register_static (GNOME_TYPE_CANVAS_ITEM,
							"glCanvasHacktext",
							&hacktext_info, 0);
	}

	return hacktext_type;
}

static void
gl_canvas_hacktext_class_init (glCanvasHacktextClass *class)
{
	GObjectClass *gobject_class;
	GtkObjectClass *object_class;
	GnomeCanvasItemClass *item_class;

	gobject_class = (GObjectClass *) class;
	object_class = (GtkObjectClass *) class;
	item_class = (GnomeCanvasItemClass *) class;

	parent_class = g_type_class_peek_parent (class);

	gobject_class->set_property = gl_canvas_hacktext_set_arg;
	gobject_class->get_property = gl_canvas_hacktext_get_arg;

	g_object_class_install_property
		(gobject_class,
		 ARG_TEXT,
		 g_param_spec_string ("text", NULL, NULL,
				      NULL,
				      (G_PARAM_READABLE | G_PARAM_WRITABLE)));

	g_object_class_install_property
		(gobject_class,
		 ARG_GLYPHLIST,
		 g_param_spec_pointer ("glyphlist", NULL, NULL,
				       (G_PARAM_READABLE | G_PARAM_WRITABLE)));

        g_object_class_install_property
                (gobject_class,
                 ARG_FILL_COLOR,
                 g_param_spec_string ("fill_color", NULL, NULL,
                                      NULL,
                                      (G_PARAM_READABLE | G_PARAM_WRITABLE)));

        g_object_class_install_property
                (gobject_class,
                 ARG_FILL_COLOR_RGBA,
                 g_param_spec_uint ("fill_color_rgba", NULL, NULL,
				    0, G_MAXUINT, 0,
				    (G_PARAM_READABLE | G_PARAM_WRITABLE)));

	g_object_class_install_property
		(gobject_class,
		 ARG_FONT,
		 g_param_spec_pointer ("font", NULL, NULL,
				       (G_PARAM_READABLE | G_PARAM_WRITABLE)));

        g_object_class_install_property
                (gobject_class,
                 ARG_X,
                 g_param_spec_double ("x", NULL, NULL,
				      -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
				      (G_PARAM_READABLE | G_PARAM_WRITABLE)));

        g_object_class_install_property
                (gobject_class,
                 ARG_Y,
                 g_param_spec_double ("y", NULL, NULL,
				      -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
				      (G_PARAM_READABLE | G_PARAM_WRITABLE)));


	object_class->destroy = gl_canvas_hacktext_destroy;

	item_class->update = gl_canvas_hacktext_update;
	item_class->realize = gl_canvas_hacktext_realize;
	item_class->unrealize = gl_canvas_hacktext_unrealize;
	item_class->point = gl_canvas_hacktext_point;
	item_class->bounds = gl_canvas_hacktext_bounds;
	item_class->render = gl_canvas_hacktext_render;
}

static void
gl_canvas_hacktext_init (glCanvasHacktext *hacktext)
{
	hacktext->text = NULL;
	hacktext->priv = g_new (glCanvasHacktextPriv, 1);
	hacktext->priv->font = NULL;
	hacktext->priv->glyphlist = NULL;
	hacktext->priv->pgl = NULL;

	art_affine_identity (hacktext->priv->affine);
}

static void
gl_canvas_hacktext_destroy (GtkObject *object)
{
	glCanvasHacktext *hacktext;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_CANVAS_HACKTEXT (object));

	hacktext = GL_CANVAS_HACKTEXT (object);

	if (hacktext->text) {
		g_free (hacktext->text);
		hacktext->text = NULL;
	}

	if (hacktext->priv) {
		if (hacktext->priv->font) gnome_font_unref (hacktext->priv->font);
		if (hacktext->priv->glyphlist) gnome_glyphlist_unref (hacktext->priv->glyphlist);
		if (hacktext->priv->pgl) gnome_pgl_destroy (hacktext->priv->pgl);
		g_free (hacktext->priv);
		hacktext->priv = NULL;
	}

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
art_drect_hacktext (ArtDRect *bbox, glCanvasHacktext *hacktext)
{
	g_assert (bbox != NULL);
	g_assert (hacktext != NULL);

	g_return_if_fail (hacktext->priv);

	if (GTK_OBJECT_FLAGS (hacktext) & GNOME_CANVAS_UPDATE_REQUESTED) {
		gnome_canvas_update_now (GNOME_CANVAS_ITEM (hacktext)->canvas);
	}

	if (!hacktext->priv->pgl) return;

	gnome_pgl_bbox (hacktext->priv->pgl, bbox);
}

/* Computes the bounding box of the hacktext.  Assumes that the number of points in the hacktext is
 * not zero.
 */
static void
get_bounds (glCanvasHacktext *hacktext, double *bx1, double *by1, double *bx2, double *by2)
{
	ArtDRect bbox;

	/* Compute bounds of hacktext */
	art_drect_hacktext (&bbox, hacktext);

	/* Done */

	*bx1 = bbox.x0;
	*by1 = bbox.y0;
	*bx2 = bbox.x1;
	*by2 = bbox.y1;
}

#ifdef IFED_OUT_BY_CHEMA_TO_KILL_COMPILE_WARNING
/* Convenience function to set a GC's foreground color to the specified pixel value */
static void
set_gc_foreground (GdkGC *gc, gulong pixel)
{
	GdkColor c;

	if (!gc)
		return;

	c.pixel = pixel;
	gdk_gc_set_foreground (gc, &c);
}
#endif

static void
gl_canvas_hacktext_set_arg (GObject *object,
			    guint arg_id,
			    const GValue *value,
			    GParamSpec *pspec)
{
	GnomeCanvasItem *item;
	glCanvasHacktext *bp;
	char *text;
	GnomeGlyphList * gl;
	GdkColor color;
	GnomeFont * font;

	item = GNOME_CANVAS_ITEM (object);
	bp = GL_CANVAS_HACKTEXT (object);

	switch (arg_id) {
	case ARG_TEXT:
		if (bp->text) {
			g_free (bp->text);
			bp->text = NULL;
		}

		if (bp->priv->glyphlist) {
			gnome_glyphlist_unref (bp->priv->glyphlist);
			bp->priv->glyphlist = NULL;
		}

		if (text) bp->text = g_value_dup_string (value);

		gnome_canvas_item_request_update (item);
		break;

	case ARG_GLYPHLIST:
		gl = g_value_get_pointer (value);

		if (bp->text) {
			g_free (bp->text);
			bp->text = NULL;
		}

		if (bp->priv->glyphlist) {
			gnome_glyphlist_unref (bp->priv->glyphlist);
			bp->priv->glyphlist = NULL;
		}

		/* fixme: should be duplicate() */

		if (gl) gnome_glyphlist_ref (gl);

		bp->priv->glyphlist = gl;

		gnome_canvas_item_request_update (item);

		break;

	case ARG_FILL_COLOR:
		if (gnome_canvas_get_color (item->canvas, g_value_get_string(value), &color)) {
			bp->fill_set = TRUE;
			bp->fill_pixel = color.pixel;
			bp->fill_rgba =
				((color.red & 0xff00) << 16) |
				((color.green & 0xff00) << 8) |
				(color.blue & 0xff00) |
				0xff;
		} else {
			bp->fill_set = FALSE;
			bp->fill_rgba = 0;
		}

		gnome_canvas_item_request_update (item);
		break;

	case ARG_FILL_COLOR_RGBA:
		bp->fill_set = TRUE;
		bp->fill_rgba = g_value_get_uint (value);

		/* should probably request repaint on the fill_svp */
		gnome_canvas_item_request_update (item);

		break;

	case ARG_FONT:
		font = g_value_get_pointer (value);
		if (font) gnome_font_ref (font);
		if (bp->priv->font) gnome_font_unref (bp->priv->font);
		bp->priv->font = font;
		bp->size = gnome_font_get_size (bp->priv->font);
		gnome_canvas_item_request_update (item);
		break;

	case ARG_X:
		bp->x = g_value_get_double (value);
		gnome_canvas_item_request_update (item);
		break;

	case ARG_Y:
		bp->y = g_value_get_double (value);
		gnome_canvas_item_request_update (item);
		break;

	default:
		break;
	}
}

static void
gl_canvas_hacktext_get_arg (GObject *object,
			    guint arg_id,
			    GValue *value,
			    GParamSpec *pspec)
{
	glCanvasHacktext *bp;

	bp = GL_CANVAS_HACKTEXT (object);

	switch (arg_id) {
	case ARG_TEXT:
		if (bp->text) {
			g_value_set_string (value, g_strdup (bp->text));
		} else
			g_value_set_string (value,  NULL);
		break;
	case ARG_FILL_COLOR_RGBA:
		g_value_set_uint (value, bp->fill_color);
		break;

	case ARG_FONT:
		g_value_set_pointer (value, bp->priv->font);
		break;

	case ARG_X:
		g_value_set_double (value, bp->x);
		break;

	case ARG_Y:
		g_value_set_double (value, bp->y);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, arg_id, pspec);
		break;
	}
}

static void
gl_canvas_hacktext_update (GnomeCanvasItem *item, double *affine, ArtSVP *clip_path, int flags)
{
	glCanvasHacktext *hacktext;
	ArtIRect ibbox = {0, 0, 0, 0};

	hacktext = (glCanvasHacktext *) item;

	if (parent_class->update)
		(* parent_class->update) (item, affine, clip_path, flags);

	if (hacktext->priv->pgl) gl_canvas_hacktext_req_repaint (hacktext, NULL);

	gnome_canvas_item_reset_bounds (item);

	hacktext->priv->affine[0] = affine[0];
	hacktext->priv->affine[1] = affine[1];
	hacktext->priv->affine[2] = -affine[2];
	hacktext->priv->affine[3] = -affine[3];
	hacktext->priv->affine[4] = affine[4] + hacktext->x * affine[0] + hacktext->y * affine[2];
	hacktext->priv->affine[5] = affine[5] + hacktext->x * affine[1] + hacktext->y * affine[3];

	if (hacktext->text) {
		if (hacktext->priv->glyphlist) {
			gnome_glyphlist_unref (hacktext->priv->glyphlist);
			hacktext->priv->glyphlist = NULL;
		}

		if (!hacktext->priv->font) return;

		hacktext->priv->glyphlist = gnome_glyphlist_from_text_dumb (hacktext->priv->font, hacktext->fill_rgba,
									    0.0, 0.0,
									    hacktext->text);
	}

	if (hacktext->priv->glyphlist) {
		GnomePosGlyphList * pgl;

		pgl = gnome_pgl_from_gl (hacktext->priv->glyphlist, hacktext->priv->affine, GNOME_PGL_RENDER_DEFAULT);

		if (hacktext->priv->pgl) gnome_pgl_destroy (hacktext->priv->pgl);

		hacktext->priv->pgl = pgl;
	}
	       
	gl_canvas_hacktext_req_repaint (hacktext, &ibbox);

	hacktext->item.x1 = ibbox.x0;
	hacktext->item.y1 = ibbox.y0;
	hacktext->item.x2 = ibbox.x1;
	hacktext->item.y2 = ibbox.y1;
}

static void
gl_canvas_hacktext_realize (GnomeCanvasItem *item)
{
	glCanvasHacktext *hacktext;

	hacktext = (glCanvasHacktext *) item;

	if (parent_class->realize)
		(* parent_class->realize) (item);
}

static void
gl_canvas_hacktext_unrealize (GnomeCanvasItem *item)
{
	glCanvasHacktext *hacktext;

	hacktext = (glCanvasHacktext *) item;

	if (parent_class->unrealize)
		(* parent_class->unrealize) (item);
}

static double
gl_canvas_hacktext_point (GnomeCanvasItem *item, double mx, double my,
			    int cx, int cy, GnomeCanvasItem **actual_item)
{
	glCanvasHacktext * hacktext;

	hacktext = (glCanvasHacktext *) item;

	if (!hacktext->priv->pgl) return 1e18;

	*actual_item = item;

	if (gnome_pgl_test_point (hacktext->priv->pgl, cx, cy)) return 0.0;

	return 1e18;
}

static void
gl_canvas_hacktext_bounds (GnomeCanvasItem *item, double *x1, double *y1, double *x2, double *y2)
{
	glCanvasHacktext *hacktext;

	g_return_if_fail (item != NULL);
	g_return_if_fail (GL_IS_CANVAS_HACKTEXT (item));

	hacktext = GL_CANVAS_HACKTEXT (item);

	if (hacktext->text == NULL) {
		*x1 = *y1 = *x2 = *y2 = 0.0;
		return;
	}

	get_bounds (hacktext, x1, y1, x2, y2);
}

static void
gl_canvas_hacktext_req_repaint (glCanvasHacktext *hacktext,
				   ArtIRect *bbox)
{
	ArtDRect gbbox;

	g_return_if_fail (hacktext->priv);

	if (!hacktext->priv->pgl) return;

	if (gnome_pgl_bbox (hacktext->priv->pgl, &gbbox)) {
		ArtIRect ibox;
		art_drect_to_irect (&ibox, &gbbox);

		/* hack: bounding box seems to be underestimated.
		 * It may be doesn't take into account antialiasing*/

		ibox.x0--;
		ibox.y0--;
		ibox.x1++;
		ibox.y1++;
		
		gnome_canvas_request_redraw (hacktext->item.canvas, ibox.x0, ibox.y0, ibox.x1, ibox.y1);
		if (bbox) art_irect_union (bbox, bbox, &ibox);
	}
}

static void
gl_canvas_hacktext_render (GnomeCanvasItem *item,
			      GnomeCanvasBuf *buf)
{
	glCanvasHacktext * hacktext;

	hacktext = (glCanvasHacktext *) item;

	g_return_if_fail (hacktext->priv);

	if (!hacktext->priv->pgl) return;

	gnome_canvas_buf_ensure_buf (buf);
	buf->is_buf = TRUE;
	buf->is_bg = FALSE;

	gnome_rfont_render_pgl_rgb8 (hacktext->priv->pgl,
				     -buf->rect.x0, -buf->rect.y0,
				     buf->buf,
				     buf->rect.x1 - buf->rect.x0,
				     buf->rect.y1 - buf->rect.y0,
				     buf->buf_rowstride,
				     GNOME_RFONT_RENDER_DEFAULT);
}












