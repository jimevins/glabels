/* Hacktext item type for GnomeCanvas widget
 *
 * GnomeCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 * Copyright (C) 1998,1999 The Free Software Foundation
 *
 * Authors: Federico Mena <federico@nuclecu.unam.mx>
 *          Raph Levien <raph@acm.org>
 */

#ifndef GNOME_CANVAS_HACKTEXT_H
#define GNOME_CANVAS_HACKTEXT_H

BEGIN_GNOME_DECLS

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

#define GNOME_TYPE_CANVAS_HACKTEXT            (gnome_canvas_hacktext_get_type ())
#define GNOME_CANVAS_HACKTEXT(obj)            (GTK_CHECK_CAST ((obj), GNOME_TYPE_CANVAS_HACKTEXT, GnomeCanvasHacktext))
#define GNOME_CANVAS_HACKTEXT_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_CANVAS_HACKTEXT, GnomeCanvasHacktextClass))
#define GNOME_IS_CANVAS_HACKTEXT(obj)         (GTK_CHECK_TYPE ((obj), GNOME_TYPE_CANVAS_HACKTEXT))
#define GNOME_IS_CANVAS_HACKTEXT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_CANVAS_HACKTEXT))

typedef struct _GnomeCanvasHacktext GnomeCanvasHacktext;
typedef struct _GnomeCanvasHacktextPriv GnomeCanvasHacktextPriv;
typedef struct _GnomeCanvasHacktextClass GnomeCanvasHacktextClass;

#include <libgnomeui/gnome-canvas.h>

struct _GnomeCanvasHacktext {
	GnomeCanvasItem item;

	char *text;			/* String of the text item */

	guint fill_color;		/* Fill color, RGBA */

	gulong fill_pixel;		/* Color for fill */

	guint fill_set : 1;		/* Is fill color set? */

	double size;			/* size in user units */

	double x, y;			/* x, y coords of text origin */

	/* Antialiased specific stuff follows */
	guint32 fill_rgba;		/* RGBA color for filling */

	GnomeCanvasHacktextPriv *priv;	/* Private data */
};

struct _GnomeCanvasHacktextClass {
	GnomeCanvasItemClass parent_class;
};


/* Standard Gtk function */
GtkType gnome_canvas_hacktext_get_type (void);


END_GNOME_DECLS

#endif
