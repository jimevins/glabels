/*  barcode_backends.vala
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

namespace glabels
{

	public class BarcodeBackends
	{

		private const string DEFAULT_ID = "Code39";
		private const string DEFAULT_NAME = _("Code 39");

		private static bool initialized = false;
		private static HashTable<string,BarcodeStyle> id_map;
		private static HashTable<string,BarcodeStyle> name_map;


		static construct
		{
			id_map   = new HashTable<string,BarcodeStyle>( str_hash, str_equal );
			name_map = new HashTable<string,BarcodeStyle>( str_hash, str_equal );

			register( "POSTNET", _("POSTNET (any)"),
			          false, false, true, false, "12345-6789-12", false, 11 );

			register( "POSTNET-5", _("POSTNET-5 (ZIP only)"),
			          false, false, true, false, "12345", false, 5 );

			register( "POSTNET-9", _("POSTNET-9 (ZIP+4)"),
			          false, false, true, false, "12345-6789", false, 9 );

			register( "POSTNET-11", _("POSTNET-11 (DPBC)"),
			          false, false, true, false, "12345-6789-12", false, 11 );

			register( "CEPNET", _("CEPNET"),
			          false, false, true, false, "12345-678", false, 8 );

			register( "ONECODE", _("One Code"),
			          false, false, true, false, "12345678901234567890", false, 20 );

			register( "Code39", _("Code 39"),
			          true, true, true, true, "1234567890", true, 10 );

			register( "Code39Ext", _("Code 39 Extended"),
			          true, true, true, true, "1234567890", true, 10 );

			register( "UPC-A", _("UPC - A"),
			          true, false, true, false, "12345678901", false, 11 );

			register( "EAN-13", _("EAN - 13"),
			          true, false, true, false, "123456789012", false, 12 );

#if HAVE_LIBBARCODE

			register( "gnu-barcode:EAN", _("GNU-Barcode : EAN (any)"),
			          true, true, true, false, "000000000000 00000", false, 17 );

			register( "gnu-barcode:EAN-8", _("GNU-Barcode : EAN-8"),
			          true, true, true, false, "0000000", false, 7 );

			register( "gnu-barcode:EAN-8+2", _("GNU-Barcode : EAN-8 +2"),
			          true, true, true, false, "0000000 00", false, 9 );

			register( "gnu-barcode:EAN-8+5", _("GNU-Barcode : EAN-8 +5"),
			          true, true, true, false, "0000000 00000", false, 12 );

			register( "gnu-barcode:EAN-13", _("GNU-Barcode : EAN-13"),
			          true, true, true, false, "000000000000", false, 12 );

			register( "gnu-barcode:EAN-13+2", _("GNU-Barcode : EAN-13 +2"),
			          true, true, true, false, "000000000000 00", false, 14 );

			register( "gnu-barcode:EAN-13+5", _("GNU-Barcode : EAN-13 +5"),
			          true, true, true, false, "000000000000 00000", false, 17 );

			register( "gnu-barcode:UPC", _("GNU-Barcode : UPC (UPC-A or UPC-E)"),
			          true, true, true, false, "00000000000 00000", false, 16 );

			register( "gnu-barcode:UPC-A", _("GNU-Barcode : UPC-A"),
			          true, true, true, false, "00000000000", false, 11 );

			register( "gnu-barcode:UPC-A+2", _("GNU-Barcode : UPC-A +2"),
			          true, true, true, false, "00000000000 00", false, 13 );

			register( "gnu-barcode:UPC-A+5", _("GNU-Barcode : UPC-A +5"),
			          true, true, true, false, "00000000000 00000", false, 16 );

			register( "gnu-barcode:UPC-E", _("GNU-Barcode : UPC-E"),
			          true, true, true, false, "000000", false, 6 );

			register( "gnu-barcode:UPC-E+2", _("GNU-Barcode : UPC-E +2"),
			          true, true, true, false, "000000 00", false, 8 );

			register( "gnu-barcode:UPC-E+5", _("GNU-Barcode : UPC-E +5"),
			          true, true, true, false, "000000 00000", false, 11 );

			register( "gnu-barcode:ISBN", _("GNU-Barcode : ISBN"),
			          true, true, true, true, "0-00000-000-0", false, 10 );

			register( "gnu-barcode:ISBN+5", _("GNU-Barcode : ISBN +5"),
			          true, true, true, true, "0-00000-000-0 00000", false, 15 );

			register( "gnu-barcode:Code39", _("GNU-Barcode : Code 39"),
			          true, true, true, true, "0000000000", true, 10 );

			register( "gnu-barcode:Code128", _("GNU-Barcode : Code 128"),
			          true, true, true, true, "0000000000", true, 10 );

			register( "gnu-barcode:Code128C", _("GNU-Barcode : Code 128C"),
			          true, true, true, false, "0000000000", true, 10 );

			register( "gnu-barcode:Code128B", _("GNU-Barcode : Code 128B"),
			          true, true, true, true, "0000000000", true, 10 );

			register( "gnu-barcode:I25", _("GNU-Barcode : Interleaved 2 of 5"),
			          true, true, true, true, "0000000000", true, 10 );

			register( "gnu-barcode:CBR", _("GNU-Barcode : Codabar"),
			          true, true, true, true, "0000000000", true, 10 );

			register( "gnu-barcode:MSI", _("GNU-Barcode : MSI"),
			          true, true, true, true, "0000000000", true, 10 );

			register( "gnu-barcode:PLS", _("GNU-Barcode : Plessey"),
			          true, true, true, true, "0000000000", true, 10 );

			register( "gnu-barcode:Code93", _("GNU-Barcode : Code 93"),
			          true, true, true, false, "0000000000", true, 10 );

#endif

#if HAVE_LIBZINT

			register( "zint:AUSP", _("zint : Australia Post Standard"),
			          false, false, true, false, "12345678901234567890123", true, 23 );

			register( "zint:AUSRP", _("zint : Australia Post Reply Paid"),
			          false, false, true, false, "12345678", true, 8 );

			register( "zint:AUSRT", _("zint : Australia Post Route Code"),
			          false, false, true, false, "12345678", true, 8 );

			register( "zint:AUSRD", _("zint : Australia Post Redirect"),
			          false, false, true, false, "12345678", true, 8 );

			register( "zint:AZTEC", _("zint : Aztec Code"),
			          false, false, true, false, "1234567890", true, 10 );
          
			register( "zint:AZRUN", _("zint : Aztec Rune"),
			          false, false, true, false, "255", true, 3 );

			register( "zint:CBR", _("zint : Codabar"),
			          true, true, true, false, "ABCDABCDAB", true, 10 );

			register( "zint:Code1", _("zint : Code One"), 
			          false, false, true, false, "0000000000", true, 10 );

			register( "zint:Code11", _("zint : Code 11"),
			          true, true, true, false, "0000000000", true, 10 );
          
			register( "zint:C16K", _("zint : Code 16K"),
			          false, false, true, false, "0000000000", true, 10 );
          
			register( "zint:C25M", _("zint : Code 2 of 5 Matrix"), 
			          true, true, true, false, "0000000000", true, 10 );
          
			register( "zint:C25I", _("zint : Code 2 of 5 IATA"), 
			          true, true, true, false, "0000000000", true, 10 );
          
			register( "zint:C25DL", _("zint : Code 2 of 5 Data Logic"), 
			          true, true, true, false, "0000000000", true, 10 );

			register( "zint:Code32", _("zint : Code 32 (Italian Pharmacode)"), 
			          true, true, true, false, "12345678", true, 8 );

			register( "zint:Code39", _("zint : Code 39"),
			          true, true, false, false, "0000000000", true, 10 );
          
			register( "zint:Code39E", _("zint : Code 39 Extended"), 
			          true, true, true, false, "0000000000", true, 10 );

			register( "zint:Code49", _("zint : Code 49"),
			          false, false, true, false, "0000000000", true, 10 );

			register( "zint:Code93", _("zint : Code 93"),
			          true, true, true, false, "0000000000", true, 10 );

			register( "zint:Code128", _("zint : Code 128"),
			          true, true, true, false, "0000000000", true, 10 );
          
			register( "zint:Code128B", _("zint : Code 128 (Mode C supression)"),
			          true, true, true, false, "0000000000", true, 10 );
          
			register( "zint:DAFT", _("zint : DAFT Code"),
			          false, false, false, false, "DAFTDAFTDAFTDAFT", true, 16 );

			register( "zint:DMTX", _("zint : Data Matrix"),
			          false, false, true, false, "0000000000", true, 10 );

			register( "zint:DPL", _("zint : Deutsche Post Leitcode"),
			          true, true, true, false, "1234567890123", true, 13 );
          
			register( "zint:DPI", _("zint : Deutsche Post Identcode"),
			          true, true, true, false, "12345678901", true, 11 );
          
			register( "zint:KIX", _("zint : Dutch Post KIX Code"),
			          false, false, true, false, "0000000000", true, 10 );

			register( "zint:EAN", _("zint : EAN"),
			          true, true, true, false, "1234567890123", false, 13 );

			register( "zint:GMTX", _("zint : Grid Matrix"), 
			          false, false, true, false, "0000000000", true, 10 );

			register( "zint:GS1-128", _("zint : GS1-128"),
			          true, true, true, false, "[01]12345678901234", false, 18 );

			register( "zint:RSS14", _("zint : GS1 DataBar-14"),
			          true, true, true, false, "1234567890123", true, 13 );
          
			register( "zint:RSSLTD", "GS1 DataBar-14 Limited", 
			          true, true, true, false, "1234567890123", true, 13 );
          
			register( "zint:RSSEXP", "GS1 DataBar Extended", 
			          true, true, true, false, "[01]12345678901234", false, 18 );
          
			register( "zint:RSSS", _("zint : GS1 DataBar-14 Stacked"),
			          false, false, true, false, "0000000000", true, 10 );

			register( "zint:RSSSO", _("zint : GS1 DataBar-14 Stacked Omni."),
			          false, false, true, false, "0000000000", true, 10 );

			register( "zint:RSSSE", _("zint : GS1 DataBar Extended Stacked"),
			          false, false, true, false, "[01]12345678901234", false, 18 );

			register( "zint:HIBC128", _("zint : HIBC Code 128"),
			          true, true, true, false, "0000000000", true, 10 );

			register( "zint:HIBC39", _("zint : HIBC Code 39"),
			          true, true, true, false, "0000000000", true, 10 );

			register( "zint:HIBCDM", _("zint : HIBC Data Matrix"),
			          false, false, true, false, "0000000000", true, 10 );

			register( "zint:HIBCQR", _("zint : HIBC QR Code"),
			          false, false, true, false, "0000000000", true, 10 );

			register( "zint:HIBCPDF", _("zint : HIBC PDF417"),
			          false, false, true, false, "0000000000", true, 10 );

			register( "zint:HIBCMPDF", _("zint : HIBC Micro PDF417"),
			          false, false, true, false, "0000000000", true, 10 );

			register( "zint:HIBCAZ", _("zint : HIBC Aztec Code"),
			          true, true, true, false, "0000000000", true, 10 );

			register( "zint:I25", _("zint : Interleaved 2 of 5"),
			          true, true, true, false, "0000000000", true, 10 );

			register( "zint:ISBN", _("zint : ISBN"),
			          true, true, true, false, "123456789", false, 9 );

			register( "zint:ITF14", _("zint : ITF-14"),
			          true, true, true, false, "0000000000", true, 10 );

			register( "zint:JAPAN", _("zint : Japanese Postal"),
			          false, false, true, false, "0000000000", true, 10 );

			register( "zint:KOREA", _("zint : Korean Postal"),
			          true, true, true, false, "123456", false, 6 );

			register( "zint:LOGM", _("zint : LOGMARS"),
			          true, true, true, false, "0000000000", true, 10 );

			register( "zint:MAXI", _("zint : Maxicode"),
			          false, false, false, false, "0000000000", true, 10 );

			register( "zint:MPDF", _("zint : Micro PDF417"),
			          false, false, true, false, "0000000000", true, 10 );

			register( "zint:MQR", _("zint : Micro QR Code"),
			          false, false, true, false, "0000000000", true, 10 );

			register( "zint:MSI", _("zint : MSI Plessey"),
			          true, true, true, false, "0000000000", true, 10 );

			register( "zint:NVE", _("zint : NVE-18"),
			          true, true, true, false, "12345678901234567", false, 17 );

			register( "zint:PDF", _("zint : PDF417"),
			          false, false, true, false, "0000000000", true, 10 );

			register( "zint:PDFT", _("zint : PDF417 Truncated"),
			          false, false, true, false, "0000000000", true, 10 );

			register( "zint:PLAN", _("zint : PLANET"),
			          false, false, true, false, "0000000000", true, 10 );

			register( "zint:POSTNET", _("zint : PostNet"),
			          true, true, true, false, "0000000000", true, 10 );

			register( "zint:PHARMA", _("zint : Pharmacode"),
			          false, false, true, false, "123456", false, 6 );

			register( "zint:PHARMA2", _("zint : Pharmacode 2-track"),
			          false, false, true, false, "12345678", false, 8 );

			register( "zint:PZN", _("zint : Pharmazentral Nummer (PZN)"),
			          true, true, true, false, "123456", false, 6 );

			register( "zint:QR", _("zint : QR Code"),
			          true, true, true, false, "0000000000", true, 10 );

			register( "zint:RM4", _("zint : Royal Mail 4-State"),
			          false, false, true, false, "0000000000", true, 10 );

			register( "zint:TELE", _("zint : Telepen"),
			          true, true, true, false, "0000000000", true, 10 );

			register( "zint:TELEX", _("zint : Telepen Numeric"),
			          true, true, true, false, "0000000000", true, 10 );

			register( "zint:UPC-A", _("zint : UPC-A"), 
			          true, true, true, false, "12345678901", false, 11 );
          
			register( "zint:UPC-E", _("zint : UPC-E"), 
			          true, true, true, false, "1234567", false, 7 );
          
			register( "zint:USPS", _("zint : USPS One Code"),
			          false, false, true, false, "12345678901234567890", true, 20 );

			register( "zint:PLS", _("zint : UK Plessey"),
			          true, true, true, false, "0000000000", true, 10 );

#endif

#if HAVE_LIBIEC16022

			register( "iec16022:IEC16022", _("IEC16022 : IEC16022 (DataMatrix)"),
			          false, false, true, false, "12345678", true, 8 );

#endif

#if HAVE_LIBQRENCODE

			register( "qrencode:IEC18004", _("qrencode : IEC18004 (QRCode)"),
			          false, false, true, false, "12345678", true, 8 );

#endif

			initialized = true;
		}


		public static void init()
		{
			if ( !initialized )
			{
				new BarcodeBackends();
			}
			else
			{
				message( "BarcodeBackends already initialized.\n" );
			}
		}


		private static void register ( string id,
		                               string name,
		                               bool   can_text,
		                               bool   text_optional,
		                               bool   can_checksum,
		                               bool   checksum_optional,
		                               string default_digits,
		                               bool   can_freeform,
		                               int    prefered_n )
		{
			BarcodeStyle style = new BarcodeStyle( id, name,
			                                       can_text, text_optional, can_checksum, checksum_optional,
			                                       default_digits, can_freeform, prefered_n );

			id_map.insert( id, style );
			name_map.insert( name, style );
		}


		public static BarcodeStyle? lookup_style_from_name( string name )
		{
			if ( name_map.contains( name ) )
			{
				return name_map.lookup( name );
			}
			else
			{
				return id_map.lookup( DEFAULT_ID );
			}
		}


		public static BarcodeStyle? lookup_style_from_id( string id )
		{
			if ( id_map.contains( id ) )
			{
				return id_map.lookup( id );
			}
			else
			{
				return id_map.lookup( DEFAULT_ID );
			}
		}


		public static string id_to_name( string id )
		{
			BarcodeStyle? style = lookup_style_from_id( id );

			if ( style != null )
			{
				return style.name;
			}
			else
			{
				return DEFAULT_NAME;
			}
		}


		public static string name_to_id( string name )
		{
			BarcodeStyle? style = lookup_style_from_name( name );

			if ( style != null )
			{
				return style.id;
			}
			else
			{
				return DEFAULT_ID;
			}
		}


		public static List<weak string> get_name_list()
		{
			List<weak string> list = name_map.get_keys();
			list.sort( strcmp );
			return list;
		}

	}

}




