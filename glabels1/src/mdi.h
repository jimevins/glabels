/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  mdi.h:  GLabels MDI module header file
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
#ifndef __MDI_H__
#define __MDI_H__

#include <gnome.h>
#include <display.h>

extern GnomeMDI *gl_mdi_init (void);
extern GnomeApp *gl_mdi_get_active_window (void);

extern gboolean gl_mdi_new_child (const gchar * tmplt_name,
				  gboolean rotate_flag);
extern gboolean gl_mdi_new_child_from_xml_file (const gchar * filename);

extern gboolean gl_mdi_save_child_to_xml_file (GnomeMDIChild * child,
					       const gchar * filename);

extern void gl_mdi_close_child (GnomeMDIChild * child);
extern void gl_mdi_close_all (void);

extern gchar *gl_mdi_get_filename (GnomeMDIChild * child);
extern gchar *gl_mdi_get_basename (GnomeMDIChild * child);

extern GList *gl_mdi_get_displays_in_current_window (void);
extern glDisplay *gl_mdi_get_display (GnomeMDIChild * child);
extern glLabel *gl_mdi_get_label (GnomeMDIChild * child);

extern gint gl_mdi_get_id (GnomeMDIChild * child);
extern GnomeMDIChild *gl_mdi_find_child (gint id);

#endif /* __MDI_H__ */
