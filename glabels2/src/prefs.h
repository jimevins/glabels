/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  prefs.h:  Application preferences module header file
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
#ifndef __PREFS_H__
#define __PREFS_H__

#include <libgnomeprint/gnome-font.h>

typedef struct _glPreferences glPreferences;

typedef enum {
	GL_TOOLBAR_SYSTEM = 0,
	GL_TOOLBAR_ICONS,
	GL_TOOLBAR_ICONS_AND_TEXT
} glToolbarSetting;

typedef enum {
	GL_PREFS_UNITS_PTS,
	GL_PREFS_UNITS_INCHES,
	GL_PREFS_UNITS_MM,
} glPrefsUnits;

struct _glPreferences
{
	/* Units */
	glPrefsUnits    units;

	/* Page size */
	gchar           *default_page_size;

	/* Text properties */
	gchar            *default_font_family;
	gdouble          default_font_size;
	GnomeFontWeight  default_font_weight;
	gboolean         default_font_italic_flag;
	guint            default_text_color;
	GtkJustification default_text_alignment;
	
	/* Line properties */
	gdouble          default_line_width;
	guint            default_line_color;
	
	/* Fill properties */
	guint            default_fill_color;

	/* User Interface/Toolbar */
	gboolean		toolbar_visible;
	glToolbarSetting 	toolbar_buttons_style; 
	gboolean		toolbar_view_tooltips;

	/* User Interface/MDI Mode */
	gint		mdi_mode;
	gint		mdi_tabs_position; /* Tabs position in mdi notebook */

	/* Recent files */
	gint            max_recents;
};

extern glPreferences *gl_prefs;

extern void gl_prefs_save_settings (void);
extern void gl_prefs_load_settings (void);
extern void gl_prefs_init (void);

extern const gchar *gl_prefs_get_page_size (void);
extern glPrefsUnits gl_prefs_get_units (void);
extern const gchar *gl_prefs_get_units_string (void);
extern gdouble gl_prefs_get_units_per_point (void);
extern gdouble gl_prefs_get_units_step_size (void);
extern gint gl_prefs_get_units_precision (void);

#endif /* __PREFS_H__ */

