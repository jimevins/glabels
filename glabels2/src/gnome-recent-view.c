#include <string.h>
#include <gtk/gtk.h>
#include "gnome-recent-view.h"


GtkType
gnome_recent_view_get_type (void)
{
	static GtkType view_type = 0;

	if (!view_type)
	{
		static const GTypeInfo view_info =
		{
			sizeof (GnomeRecentViewClass),  /* class_size */
			NULL,			    /* base_init */
			NULL,			    /* base_finalize */
		};

		view_type = g_type_register_static (G_TYPE_INTERFACE,
						    "GnomeRecentView",
						    &view_info, 0);
	}

	return view_type;
}

void
gnome_recent_view_clear (GnomeRecentView *view)
{
	g_return_if_fail (GNOME_IS_RECENT_VIEW (view));
  
	GNOME_RECENT_VIEW_GET_CLASS (view)->do_clear (view);
}

GnomeRecentModel *
gnome_recent_view_get_model (GnomeRecentView *view)
{
	g_return_val_if_fail (view, NULL);

	return GNOME_RECENT_VIEW_GET_CLASS (view)->do_get_model (view);
}

void
gnome_recent_view_set_model (GnomeRecentView *view, GnomeRecentModel *model)
{
	g_return_if_fail (view);
	g_return_if_fail (model);

	GNOME_RECENT_VIEW_GET_CLASS (view)->do_set_model (view, model);
}
