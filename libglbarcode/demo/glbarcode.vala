/*  glbarcode.vala
 *
 *  Copyright (C) 2012  Jim Evins <evins@snaught.com>
 *
 *  This file is part of libglbarcode.
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

namespace glbarcode
{

	class Demo
	{

		private static string type;
		private static bool   text;
		private static bool   checksum;
		private static double w;
		private static double h;
		private static string data;
		private static double ppi;

		private const OptionEntry[] option_entries = {
			{ "type",      't', 0, OptionArg.STRING, ref type,
			  "barcode type",
			  "TYPE"
			},
			{ "text", 'x', 0, OptionArg.NONE, ref text,
			  "Show literal text (if supported)",
			  null
			},
			{ "checksum", 'c', 0, OptionArg.NONE, ref checksum,
			  "Include checksum (if optional)",
			  null
			},
			{ "width", 'w', 0, OptionArg.DOUBLE, ref w,
			  "Suggested width (points)",
			  "WIDTH"
			},
			{ "height", 'h', 0, OptionArg.DOUBLE, ref h,
			  "Suggested height (points)",
			  "HEIGHT"
			},
			{ "data", 'd', 0, OptionArg.STRING, ref data,
			  "Barcode data",
			  "DATA"
			},
			{ "resolution", 'r', 0, OptionArg.DOUBLE, ref ppi,
			  "Resolution (Pixels per Inch)",
			  "PPI"
			},
			{ null }
		};


		internal static int main( string[] args )
		{
			/* Set defaults. */
			type      = "Code39";
			text      = false;
			checksum  = false;
			w         = 144;
			h         = 72;
			data      = "";
			ppi       = 72;

			/* Parse command line arguments. */
			var option_context = new OptionContext( "" );
			option_context.set_summary( "Command line demo program for libglbarcode." );
			option_context.add_main_entries( option_entries, null );

			try {
				option_context.parse( ref args );
			}
			catch ( OptionError e )
			{
				message( "%s\nRun '%s --help' to see a full list of available command line options.\n",
				         e.message, args[0] );
				return -1;
			}

			/* Initialize barcode factory. */
			glbarcode.Factory.init();

			/* Create barcode object. */
			glbarcode.Barcode barcode = glbarcode.Factory.create_barcode( type, text, checksum, w, h, data );

			/* Create and initialize cairo surface and context to render to. */
			double scale = ppi / glbarcode.Constants.PTS_PER_INCH;
			var surface = new Cairo.ImageSurface( Cairo.Format.ARGB32,
			                                      (int)(scale*barcode.w),
			                                      (int)(scale*barcode.h) );
			var cr = new Cairo.Context( surface );
			cr.scale( scale, scale );
			cr.rectangle( 0, 0, barcode.w, barcode.h );
			cr.set_source_rgba( 1, 1, 1, 1 );
			cr.fill();

			/* Render barcode to cairo surface. */
			cr.set_source_rgba( 0, 0, 0, 1 );
			barcode.render( new glbarcode.CairoRenderer( cr ) );

			/* Write surface to output file. */
			message( "Creating output.png\n" );
			surface.write_to_png( "output.png" );

			return 0;
		}

	}

}


