/*  template_frame.vala
 *
 *  Copyright (C) 2011  Jim Evins <evins@snaught.com>
 *
 *  This file is part of libglabels.
 *
 *  libglabels is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libglabels is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libglabels.  If not, see <http://www.gnu.org/licenses/>.
 */


using GLib;

namespace libglabels
{

	public abstract class TemplateFrame
	{
		public string      id    { get; protected set; default = "0"; }

		public unowned List<TemplateLayout> layouts { get; protected set; }
		public unowned List<TemplateMarkup> markups { get; protected set; }


		public abstract TemplateFrame dup();

		public abstract void get_size( out double w, out double h );

		public abstract bool is_similar( TemplateFrame frame2 );

		public abstract string get_size_description( Units units );

		public abstract void cairo_path( Cairo.Context cr,
		                                 bool          waste_flag );


		public int get_n_labels()
		{
			int n_labels = 0;

			foreach ( TemplateLayout layout in layouts )
			{
				n_labels += layout.nx * layout.ny;
			}

			return n_labels;
		}


		public string get_layout_description()
		{
			string description;
			int    n_labels = get_n_labels();

			if ( layouts.length() == 1 )
			{
				TemplateLayout layout = layouts.first().data;

				/*
				 * Translators: 1st %d = number of labels across a page,
				 *              2nd %d = number of labels down a page,
				 *              3rd %d = total number of labels on a page (sheet).
				 */
				description = _("%d × %d (%d per sheet)").printf( layout.nx, layout.ny, n_labels);
			}
			else
			{
				/* Translators: %d is the total number of labels on a page (sheet). */
				description = _("%d per sheet").printf( n_labels );
			}

			return description;
		}


		public Gee.ArrayList<TemplateCoord?> get_origins()
		{
			Gee.ArrayList<TemplateCoord?> origins = new Gee.ArrayList<TemplateCoord?>();

			foreach (TemplateLayout layout in layouts)
			{
				for ( int iy = 0; iy < layout.ny; iy++ )
				{
					for ( int ix = 0; ix < layout.nx; ix++ )
					{
						origins.add( TemplateCoord( ix*layout.dx + layout.x0,
						                            iy*layout.dy + layout.y0 ) );
					}
				}
			}

			origins.sort( (CompareFunc<TemplateCoord>)TemplateCoord.compare_to );

			return origins;
		}


		public void add_layout( TemplateLayout layout )
		{
			layouts.append( layout );
		}


		public void add_markup( TemplateMarkup markup )
		{
			markups.append( markup );
		}


	}


}
