/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  print.h:  Print module header file
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
#ifndef __PRINT_H__
#define __PRINT_H__

#include <gnome.h>
#include <libgnomeprint/gnome-print-master.h>
#include <libgnomeprintui/gnome-print-master-preview.h>
#include "label.h"

G_BEGIN_DECLS

void gl_print_simple           (GnomePrintMaster *master,
				glLabel          *label,
				gint              n_sheets,
				gint              first,
				gint              last,
				gboolean          outline_flag,
				gboolean          reverse_flag);

void gl_print_merge_collated   (GnomePrintMaster *master,
				glLabel          *label,
				GList            *record_list,
				gint              n_copies,
				gint              first,
				gboolean          outline_flag,
				gboolean          reverse_flag);

void gl_print_merge_uncollated (GnomePrintMaster *master,
				glLabel          *label,
				GList            *record_list,
				gint              n_copies,
				gint              first,
				gboolean          outline_flag,
				gboolean          reverse_flag);

void gl_print_batch            (GnomePrintMaster *master,
				glLabel          *label,
				gint              n_sheets,
				gint              n_bcopies,
				gboolean          outline_flag,
				gboolean          reverse_flag);

G_END_DECLS

#endif
