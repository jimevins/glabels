#ifndef __GL_CANVAS_HACKTEXT_H__
#define __GL_CANVAS_HACKTEXT_H__

/* Hacktext item type for GnomeCanvas widget
 *
 * GnomeCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 * Copyright (C) 1998,1999 The Free Software Foundation
 *
 * Authors: Federico Mena <federico@nuclecu.unam.mx>
 *          Raph Levien <raph@acm.org>
 *
 * Modified by Jim Evins <evins@snaught.com> for gLabels.
 */

#include <glib.h>

G_BEGIN_DECLS

/* Hacktext item for the canvas.  The API is totally unstable - it needs to be replaced with one
 * that supports Unicode and the merged GnomeText/GScript API. However, I need a text item now,
 * and the GnomeText/GScript integration is going to take a bit more effort.
 *
 * The following object arguments are available:
 *
 * name			type			read/write	description
 * ------------------------------------------------------------------------------------------
 * text			char *			RW		The string of the text item.
 * glyphlist            GnomeGlyphList *        W               Glyphlist
 * fill_color		string			W		X color specification for fill color,
 *								or NULL pointer for no color (transparent).
 * fill_color_gdk	GdkColor*		RW		Allocated GdkColor for fill.
 */

#define GL_TYPE_CANVAS_HACKTEXT (gl_canvas_hacktext_get_type ())
#define GL_CANVAS_HACKTEXT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_CANVAS_HACKTEXT, glCanvasHacktext))
#define GL_CANVAS_HACKTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_CANVAS_HACKTEXT, glCanvasHacktextClass))
#define GL_IS_CANVAS_HACKTEXT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_CANVAS_HACKTEXT))
#define GL_IS_CANVAS_HACKTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_CANVAS_HACKTEXT))

typedef struct _glCanvasHacktext glCanvasHacktext;
typedef struct _glCanvasHacktextPriv glCanvasHacktextPriv;
typedef struct _glCanvasHacktextClass glCanvasHacktextClass;

#include <libgnomecanvas/libgnomecanvas.h>

struct _glCanvasHacktext {
	GnomeCanvasItem item;

	char *text;			/* String of the text item */
	guint fill_color;		/* Fill color, RGBA */
	gulong fill_pixel;		/* Color for fill */
	guint fill_set : 1;		/* Is fill color set? */

	double size;			/* size in user units */
	double x, y;			/* x, y coords of text origin */
	/* Antialiased specific stuff follows */
	guint32 fill_rgba;		/* RGBA color for filling */
	glCanvasHacktextPriv *priv;	/* Private data */
};

struct _glCanvasHacktextClass {
	GnomeCanvasItemClass parent_class;
};


GType gl_canvas_hacktext_get_type (void);


G_END_DECLS

#endif
