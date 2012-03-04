/*  xml_util.vala
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

	public class XmlUtil
	{
		public static Units default_units { get; set; }


		public static void init()
		{
			/* Force construction of static fields. */
			new XmlUtil();
		}


		static construct {
			default_units = Units.point();
		}


		public static string? get_prop_string( Xml.Node node,
		                                       string   property,
		                                       string?  default_val )
		{
			string? val;

			val = node.get_prop( property );
			if ( val != null )
			{
				return val;
			}

			return default_val;
		}


		public static string? get_prop_i18n_string( Xml.Node node,
		                                            string   property,
		                                            string?  default_val )
		{
			string  i18n_property = "_%s".printf( property );
			string? val;

			val = node.get_prop( i18n_property );

			if ( val != null )
			{
				return dgettext( null, val );
			}

			val = node.get_prop( property );
			if ( val != null )
			{
				return val;
			}

			return default_val;
		}


		public static double get_prop_double( Xml.Node node,
		                                      string   property,
		                                      double   default_val )
		{
			string? val_string;

			val_string = node.get_prop( property );
			if ( val_string != null )
			{
				return double.parse( val_string );
			}

			return default_val;
		}


		public static bool get_prop_bool( Xml.Node node,
		                                  string   property,
		                                  bool     default_val )
		{
			string? val_string;

			val_string = node.get_prop( property );
			if ( val_string != null )
			{
				switch (val_string)
				{
				case "True":
				case "TRUE":
				case "true":
				case "1":
					return true;

				default:
					return bool.parse( val_string );
				}
			}

			return default_val;
		}


		public static int get_prop_int( Xml.Node node,
		                                string   property,
		                                int      default_val )
		{
			string? val_string;

			val_string = node.get_prop( property );
			if ( val_string != null )
			{
				return int.parse( val_string );
			}

			return default_val;
		}


		public static uint get_prop_uint( Xml.Node node,
		                                  string   property,
		                                  uint     default_val )
		{
			string? val_string;

			val_string = node.get_prop( property );
			if ( val_string != null )
			{
				return (uint)uint64.parse( val_string );
			}

			return default_val;
		}


		public static double get_prop_length( Xml.Node node,
		                                      string   property,
		                                      double   default_val )
		{
			string? val_string;

			val_string = node.get_prop( property );
			if ( val_string != null )
			{
				double val;
				string units_id = string.nfill( val_string.length, 0 );

				val_string.scanf( "%lg%s", out val, units_id );

				Units units = Units.from_id( units_id );
				val *= units.points_per_unit;

				return val;
			}

			return default_val;
		}


		public static void set_prop_string( Xml.Node node,
		                                    string   property,
		                                    string?  val )
		{
			if ( val != null )
			{
				node.set_prop( property, val );
			}
		}


		public static void set_prop_double( Xml.Node node,
		                                    string   property,
		                                    double   val )
		{
			string val_string = "%g".printf( val );
			node.set_prop( property, val_string );
		}


		public static void set_prop_bool( Xml.Node node,
		                                  string   property,
		                                  bool   val )
		{
			node.set_prop( property, val ? "true" : "false" );
		}


		public static void set_prop_int( Xml.Node node,
		                                 string   property,
		                                 int      val )
		{
			string val_string = "%d".printf( val );
			node.set_prop( property, val_string );
		}


		public static void set_prop_uint_hex( Xml.Node node,
		                                      string   property,
		                                      uint     val )
		{
			string val_string = "0x%08x".printf( val );
			node.set_prop( property, val_string );
		}


		public static void set_prop_length( Xml.Node node,
		                                    string   property,
		                                    double   val )
		{
			string units_id = default_units.id;
			double units_per_point = default_units.units_per_point;

			val *= units_per_point;

			string val_string = "%g%s".printf( val, units_id );
			node.set_prop( property, val_string );
		}


	}

}