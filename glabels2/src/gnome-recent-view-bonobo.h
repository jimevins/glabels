/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef __GNOME_RECENT_VIEW_BONOBO_H__
#define __GNOME_RECENT_VIEW_BONOBO_H__

#include <libbonoboui.h>

G_BEGIN_DECLS

#define GNOME_RECENT_VIEW_BONOBO(obj)		G_TYPE_CHECK_INSTANCE_CAST (obj, gnome_recent_view_bonobo_get_type (), GnomeRecentViewBonobo)
#define GNOME_RECENT_VIEW_BONOBO_CLASS(klass) 	G_TYPE_CHECK_CLASS_CAST (klass, gnome_recent_view_bonobo_get_type (), GnomeRecentViewBonoboClass)
#define GNOME_IS_RECENT_VIEW_BONOBO(obj)		G_TYPE_CHECK_INSTANCE_TYPE (obj, gnome_recent_view_bonobo_get_type ())

typedef struct _GnomeRecentViewBonobo GnomeRecentViewBonobo;

typedef struct _GnomeRecentViewBonoboClass GnomeRecentViewBonoboClass;

GType        gnome_recent_view_bonobo_get_type (void);

GnomeRecentViewBonobo * gnome_recent_view_bonobo_new (BonoboUIComponent *uic,
						      const gchar *path);


void gnome_recent_view_bonobo_set_ui_component (GnomeRecentViewBonobo *view,
						BonoboUIComponent *uic);

void gnome_recent_view_bonobo_set_ui_path      (GnomeRecentViewBonobo *view,
						const gchar *path);

gchar * gnome_recent_view_bonobo_get_ui_path   (GnomeRecentViewBonobo *view);
BonoboUIComponent *gnome_recent_view_bonobo_get_ui_component (GnomeRecentViewBonobo *view);

G_END_DECLS

#endif /* __GNOME_RECENT_VIEW_BONOBO_H__ */
