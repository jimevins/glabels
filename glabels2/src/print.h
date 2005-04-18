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

#include <libgnomeprint/gnome-print-job.h>

#include "label.h"

G_BEGIN_DECLS

typedef struct {
	gboolean outline;
	gboolean reverse;
	gboolean crop_marks;
} glPrintFlags;

void gl_print_simple           (GnomePrintJob    *job,
				glLabel          *label,
				gint              n_sheets,
				gint              first,
				gint              last,
				glPrintFlags     *flags);

void gl_print_merge_collated   (GnomePrintJob    *job,
				glLabel          *label,
				gint              n_copies,
				gint              first,
				glPrintFlags     *flags);

void gl_print_merge_uncollated (GnomePrintJob    *job,
				glLabel          *label,
				gint              n_copies,
				gint              first,
				glPrintFlags     *flags);

void gl_print_batch            (GnomePrintJob    *job,
				glLabel          *label,
				gint              n_sheets,
				gint              n_copies,
				gint              first,
				glPrintFlags     *flags);

G_END_DECLS

#endif
