/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  bc_qrcode.h:  front-end to qrcode-library module header file
 *
 *  Copyright (C) 2001-2006  Jim Evins <evins@snaught.com>.
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
#ifndef __BC_IEC18004_H__
#define __BC_IEC18004_H__

#include "bc.h"

G_BEGIN_DECLS

glBarcode *gl_barcode_iec18004_new (const gchar    *id,
                                    gboolean        text_flag,
                                    gboolean        checksum_flag,
                                    gdouble         w,
                                    gdouble         h,
                                    const gchar    *digits);

G_END_DECLS

#endif /* __BC_IEC18004_H__ */
