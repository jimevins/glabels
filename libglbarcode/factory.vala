/*  factory.vala
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

	private uint str_case_hash( string s )
	{
		uint h = 5381;

		for ( int i = 0; i < s.length; i++ )
		{
			h = (h << 5) + h + s[i].tolower();
		}
		return h;
	}


	private bool str_case_equal( string a, string b )
	{
		return a.ascii_casecmp( b ) == 0;
	}


	/**
	 * Barcode factory.
	 */
	public class Factory
	{
		private static bool initialized = false;
		private static HashTable<string,Type> mappings;


		static construct
		{
			mappings = new HashTable<string, Type>( str_case_hash, str_case_equal );

			/* Register built-in types. */
			register_type( "Code39",          typeof(BarcodeCode39) );
			register_type( "Code39Ext",       typeof(BarcodeCode39Extended) );
			register_type( "ONECODE",         typeof(BarcodeOneCode) );
			register_type( "POSTNET",         typeof(BarcodePostnet) );
			register_type( "POSTNET-5",       typeof(BarcodePostnet5) );
			register_type( "POSTNET-9",       typeof(BarcodePostnet9) );
			register_type( "POSTNET-11",      typeof(BarcodePostnet11) );
			register_type( "CEPNET",          typeof(BarcodeCepnet) );
			register_type( "UPC-A",           typeof(BarcodeUPCA) );
			register_type( "EAN-13",          typeof(BarcodeEAN13) );

			initialized = true;
		}


		/**
		 * Initialize barcode factory.
		 *
		 * Initializes the singleton barcode factory.
		 */
		public static void init()
		{
			if ( !initialized )
			{
				new Factory();
			}
			else
			{
				message( "glbarcode.Factory already initialized.\n" );
			}
		}


		/**
		 * Register barcode type with factory.
		 *
		 * @param name barcode type, e.g. "Code39"
		 * @param type barcode object class type, e.g. typeof(BarcodeCode39).  Must be a subtype of Barcode.
		 */
		public static void register_type( string name,
		                                  Type   type )
		{
			if ( type.is_a( typeof(Barcode) ) )
			{
				mappings.insert( name, type );
			}
			else
			{
				message( "glBarcode.Factory.register_type: invalid barcode type: %s\n", type.name() );
			}
		}


		/**
		 * Create a new barcode object.
		 *
		 * @param name barcode type, e.g. "Code39"
		 * @param text_flag show literal text under barcode
		 * @param checksum_flag include a checksum
		 * @param w suggested width of barcode
		 * @param h suggested height of barcode
		 * @param data data to encode in barcode
		 */
		public static Barcode? create_barcode( string  name,
		                                       bool    text_flag,
		                                       bool    checksum_flag,
		                                       double  w,
		                                       double  h,
		                                       string? data )
		{
			Barcode? barcode = null;

			Type type = mappings.lookup( name );

			if ( mappings.contains( name ) )
			{
				barcode = Object.new( type,
				                      text_flag     : text_flag,
				                      checksum_flag : checksum_flag,
				                      w             : w,
				                      h             : h,
				                      data          : data ) as Barcode;
			}
			else
			{
				message( "Unknown barcode type: \"%s\"\n", name );
			}

			return barcode;
		}


	}

}
