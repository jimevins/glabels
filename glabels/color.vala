/*  color.vala
 *
 *  Copyright (C) 2011  Jim Evins <evins@snaught.com>
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

namespace glabels
{

	public struct Color
	{
		public double r { get; set; }
		public double g { get; set; }
		public double b { get; set; }
		public double a { get; set; }

		public Color( double r, double g, double b, double a )
		{
			this.r = r;
			this.g = g;
			this.b = b;
			this.a = a;
		}

		public Color.from_rgba( double r, double g, double b, double a )
		{
			this( r, g, b, a );
		}

		public Color.from_rgb( double r, double g, double b )
		{
			this( r, g, b, 1.0 );
		}

		public Color.from_byte_rgba( uchar r, uchar g, uchar b, uchar a )
		{
			this( r/255.0, g/255.0, b/255.0, a/255.0 );
		}

		public Color.from_byte_rgb( uchar r, uchar g, uchar b )
		{
			this( r/255.0, g/255.0, b/255.0, 1.0 );
		}

		public Color.from_gdk_color( Gdk.Color c )
		{
			this( c.red/65535.0, c.green/65535.0, c.blue/65535.0, 1.0 );
		}

		public Color.from_legacy_color( uint32 c )
		{
			this( ((c>>24) & 0xff) / 255.0,
			      ((c>>16) & 0xff) / 255.0,
			      ((c>>8)  & 0xff) / 255.0,
			      ((c)     & 0xff) / 255.0 );
		}

		public Color.none()
		{
			this( 0, 0, 0, 0 );
		}

		public Color.black()
		{
			this( 0, 0, 0, 1 );
		}

		public Color.white()
		{
			this( 1, 1, 1, 1 );
		}

		public Color.from_color_and_opacity( Color color,
		                                     double opacity )
		{
			this( color.r, color.g, color.b, opacity * color.a );
		}

		public Gdk.Color to_gdk_color()
		{
			Gdk.Color c = Gdk.Color();

			c.red   = (uint16) (r * 65535);
			c.green = (uint16) (g * 65535);
			c.blue  = (uint16) (b * 65535);

			return c;
		}

		public uint32 to_legacy_color()
		{
			uint32 c;

			c = (((uint32)(r*255) & 0xff) << 24) |
			    (((uint32)(g*255) & 0xff) << 16) |
			    (((uint32)(b*255) & 0xff) << 8)  |
			    (((uint32)(a*255) & 0xff));

			return c;
		}

		public void set_opacity( double opacity )
		{
			a *= opacity;
		}

		public bool equal( Color c )
		{
			return ( (this.r == c.r) && (this.g == c.g) && (this.b == c.b) && (this.a == c.a) );
		}

		public bool has_alpha()
		{
			return ( a != 0 );
		}

	}
	
}
