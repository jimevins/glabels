/*  print_op.vala
 *
 *  Copyright (C) 2012  Jim Evins <evins@snaught.com>
 *
 *  This file is part of gLabels.
 *
 *  gLabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gLabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gLabels.  If not, see <http://www.gnu.org/licenses/>.
 */


using GLib;
using libglabels;

namespace glabels
{

	public class PrintOp : Gtk.PrintOperation
	{
		public Model     model       { get; private set; }


		public PrintOp( Model model )
		{
			this.model = model;

			set_page_size();

			begin_print.connect( on_begin_print );
			draw_page.connect( on_draw_page );
		}


		private void set_page_size()
		{
			Paper? paper = Db.lookup_paper_from_id( model.label.template.paper_id );

			Gtk.PaperSize psize;
			if ( paper == null )
			{
				string name = Gtk.PaperSize.get_default();
				psize = new Gtk.PaperSize( name );
			}
			else if ( Db.is_paper_id_other( paper.id ) )
			{
				psize = new Gtk.PaperSize.custom( paper.id, paper.name,
				                                  model.label.template.page_width, model.label.template.page_height,
				                                  Gtk.Unit.POINTS );
			}
			else
			{
				psize = new Gtk.PaperSize( paper.pwg_size );
			}

			Gtk.PageSetup su = new Gtk.PageSetup();
			su.set_paper_size( psize );
			set_default_page_setup( su );
		}


		private void on_begin_print( Gtk.PrintContext context )
		{
			set_n_pages( model.print.n_pages );
		}


		private void on_draw_page( Gtk.PrintContext context, int i_page )
		{
			Cairo.Context cr = context.get_cairo_context();

			if ( model.label.merge is MergeNone )
			{
				model.print.print_simple_sheet( cr, i_page );
			}
		}


	}

}

