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


		private static Gee.HashMap<string,string> backend_id_map;
		private static Gee.HashMap<string,string> backend_name_map;

		private static Gee.HashMap<string,BarcodeStyle> id_map;
		private static Gee.HashMap<string,BarcodeStyle> name_map;

		private static bool initialized = false;


		static construct
		{
			backend_id_map   = new Gee.HashMap<string,string>();
			backend_name_map = new Gee.HashMap<string,string>();

			id_map   = new Gee.HashMap<string,BarcodeStyle>();
			name_map = new Gee.HashMap<string,BarcodeStyle>();

			register_style( "POSTNET", "", _("POSTNET (any)"),
			                false, false, true, false, "12345-6789-12", false, 11 );

			register_style( "POSTNET-5", "", _("POSTNET-5 (ZIP only)"),
			                false, false, true, false, "12345", false, 5 );

			register_style( "POSTNET-9", "", _("POSTNET-9 (ZIP+4)"),
			                false, false, true, false, "12345-6789", false, 9 );

			register_style( "POSTNET-11", "", _("POSTNET-11 (DPBC)"),
			                false, false, true, false, "12345-6789-12", false, 11 );

			register_style( "CEPNET", "", _("CEPNET"),
			                false, false, true, false, "12345-678", false, 8 );

			register_style( "ONECODE", "", _("USPS Intelligent Mail"),
			                false, false, true, false, "12345678901234567890", false, 20 );

			register_style( "Code39", "", _("Code 39"),
			                true, true, true, true, "1234567890", true, 10 );

			register_style( "Code39Ext", "", _("Code 39 Extended"),
			                true, true, true, true, "1234567890", true, 10 );

			register_style( "UPC-A", "", _("UPC-A"),
			                true, false, true, false, "12345678901", false, 11 );

			register_style( "EAN-13", "", _("EAN-13"),
			                true, false, true, false, "123456789012", false, 12 );

#if HAVE_LIBBARCODE

			register_backend( "gnu-barcode", _("GNU Barcode") );

			glbarcode.Factory.register_type( "gnu-barcode:EAN",      typeof(BackendGnuBarcode.Ean)      );
			glbarcode.Factory.register_type( "gnu-barcode:EAN-8",    typeof(BackendGnuBarcode.Ean8)     );
			glbarcode.Factory.register_type( "gnu-barcode:EAN-8+2",  typeof(BackendGnuBarcode.Ean8_2)   );
			glbarcode.Factory.register_type( "gnu-barcode:EAN-8+5",  typeof(BackendGnuBarcode.Ean8_5)   );
			glbarcode.Factory.register_type( "gnu-barcode:EAN-13",   typeof(BackendGnuBarcode.Ean13)    );
			glbarcode.Factory.register_type( "gnu-barcode:EAN-13+2", typeof(BackendGnuBarcode.Ean13_2)  );
			glbarcode.Factory.register_type( "gnu-barcode:EAN-13+5", typeof(BackendGnuBarcode.Ean13_5)  );
			glbarcode.Factory.register_type( "gnu-barcode:UPC",      typeof(BackendGnuBarcode.Upc)      );
			glbarcode.Factory.register_type( "gnu-barcode:UPC-A",    typeof(BackendGnuBarcode.UpcA)     );
			glbarcode.Factory.register_type( "gnu-barcode:UPC-A+2",  typeof(BackendGnuBarcode.UpcA_2)   );
			glbarcode.Factory.register_type( "gnu-barcode:UPC-A+5",  typeof(BackendGnuBarcode.UpcA_5)   );
			glbarcode.Factory.register_type( "gnu-barcode:UPC-E",    typeof(BackendGnuBarcode.UpcE)     );
			glbarcode.Factory.register_type( "gnu-barcode:UPC-E+2",  typeof(BackendGnuBarcode.UpcE_2)   );
			glbarcode.Factory.register_type( "gnu-barcode:UPC-E+5",  typeof(BackendGnuBarcode.UpcE_5)   );
			glbarcode.Factory.register_type( "gnu-barcode:ISBN",     typeof(BackendGnuBarcode.Isbn)     );
			glbarcode.Factory.register_type( "gnu-barcode:ISBN+5",   typeof(BackendGnuBarcode.Isbn_5)   );
			glbarcode.Factory.register_type( "gnu-barcode:Code39",   typeof(BackendGnuBarcode.Code39)   );
			glbarcode.Factory.register_type( "gnu-barcode:Code128",  typeof(BackendGnuBarcode.Code128)  );
			glbarcode.Factory.register_type( "gnu-barcode:Code128C", typeof(BackendGnuBarcode.Code128C) );
			glbarcode.Factory.register_type( "gnu-barcode:Code128B", typeof(BackendGnuBarcode.Code128B) );
			glbarcode.Factory.register_type( "gnu-barcode:I25",      typeof(BackendGnuBarcode.I25)      );
			glbarcode.Factory.register_type( "gnu-barcode:CBR",      typeof(BackendGnuBarcode.Cbr)      );
			glbarcode.Factory.register_type( "gnu-barcode:MSI",      typeof(BackendGnuBarcode.Msi)      );
			glbarcode.Factory.register_type( "gnu-barcode:PLS",      typeof(BackendGnuBarcode.Pls)      );
			glbarcode.Factory.register_type( "gnu-barcode:Code93",   typeof(BackendGnuBarcode.Code93)   );

			register_style( "gnu-barcode:EAN", "gnu-barcode", _("EAN (any)"),
			                true, true, true, false, "000000000000 00000", false, 17 );

			register_style( "gnu-barcode:EAN-8", "gnu-barcode", _("EAN-8"),
			                true, true, true, false, "0000000", false, 7 );

			register_style( "gnu-barcode:EAN-8+2", "gnu-barcode", _("EAN-8 +2"),
			                true, true, true, false, "0000000 00", false, 9 );

			register_style( "gnu-barcode:EAN-8+5", "gnu-barcode", _("EAN-8 +5"),
			                true, true, true, false, "0000000 00000", false, 12 );

			register_style( "gnu-barcode:EAN-13", "gnu-barcode", _("EAN-13"),
			                true, true, true, false, "000000000000", false, 12 );

			register_style( "gnu-barcode:EAN-13+2", "gnu-barcode", _("EAN-13 +2"),
			                true, true, true, false, "000000000000 00", false, 14 );

			register_style( "gnu-barcode:EAN-13+5", "gnu-barcode", _("EAN-13 +5"),
			                true, true, true, false, "000000000000 00000", false, 17 );

			register_style( "gnu-barcode:UPC", "gnu-barcode", _("UPC (UPC-A or UPC-E)"),
			                true, true, true, false, "00000000000 00000", false, 16 );

			register_style( "gnu-barcode:UPC-A", "gnu-barcode", _("UPC-A"),
			                true, true, true, false, "00000000000", false, 11 );

			register_style( "gnu-barcode:UPC-A+2", "gnu-barcode", _("UPC-A +2"),
			                true, true, true, false, "00000000000 00", false, 13 );

			register_style( "gnu-barcode:UPC-A+5", "gnu-barcode", _("UPC-A +5"),
			                true, true, true, false, "00000000000 00000", false, 16 );

			register_style( "gnu-barcode:UPC-E", "gnu-barcode", _("UPC-E"),
			                true, true, true, false, "000000", false, 6 );

			register_style( "gnu-barcode:UPC-E+2", "gnu-barcode", _("UPC-E +2"),
			                true, true, true, false, "000000 00", false, 8 );

			register_style( "gnu-barcode:UPC-E+5", "gnu-barcode", _("UPC-E +5"),
			                true, true, true, false, "000000 00000", false, 11 );

			register_style( "gnu-barcode:ISBN", "gnu-barcode", _("ISBN"),
			                true, true, true, true, "0-00000-000-0", false, 10 );

			register_style( "gnu-barcode:ISBN+5", "gnu-barcode", _("ISBN +5"),
			                true, true, true, true, "0-00000-000-0 00000", false, 15 );

			register_style( "gnu-barcode:Code39", "gnu-barcode", _("Code 39"),
			                true, true, true, true, "0000000000", true, 10 );

			register_style( "gnu-barcode:Code128", "gnu-barcode", _("Code 128"),
			                true, true, true, true, "0000000000", true, 10 );

			register_style( "gnu-barcode:Code128C", "gnu-barcode", _("Code 128C"),
			                true, true, true, false, "0000000000", true, 10 );

			register_style( "gnu-barcode:Code128B", "gnu-barcode", _("Code 128B"),
			                true, true, true, true, "0000000000", true, 10 );

			register_style( "gnu-barcode:I25", "gnu-barcode", _("Interleaved 2 of 5"),
			                true, true, true, true, "0000000000", true, 10 );

			register_style( "gnu-barcode:CBR", "gnu-barcode", _("Codabar"),
			                true, true, true, true, "0000000000", true, 10 );

			register_style( "gnu-barcode:MSI", "gnu-barcode", _("MSI"),
			                true, true, true, true, "0000000000", true, 10 );

			register_style( "gnu-barcode:PLS", "gnu-barcode", _("Plessey"),
			                true, true, true, true, "0000000000", true, 10 );

			register_style( "gnu-barcode:Code93", "gnu-barcode", _("Code 93"),
			                true, true, true, false, "0000000000", true, 10 );

#endif

#if HAVE_LIBZINT

			register_backend( "zint", _("Zint") );

			glbarcode.Factory.register_type( "zint:AUSP",     typeof(BackendZint.AusP)     );
			glbarcode.Factory.register_type( "zint:AUSRP",    typeof(BackendZint.AusRP)    );
			glbarcode.Factory.register_type( "zint:AUSRT",    typeof(BackendZint.AusRT)    );
			glbarcode.Factory.register_type( "zint:AUSRD",    typeof(BackendZint.AusRD)    );
			glbarcode.Factory.register_type( "zint:AZTEC",    typeof(BackendZint.Aztec)    );
			glbarcode.Factory.register_type( "zint:AZRUN",    typeof(BackendZint.Azrun)    );
			glbarcode.Factory.register_type( "zint:CBR",      typeof(BackendZint.Cbr)      );
			glbarcode.Factory.register_type( "zint:Code1",    typeof(BackendZint.Code1)    );
			glbarcode.Factory.register_type( "zint:Code11",   typeof(BackendZint.Code11)   );
			glbarcode.Factory.register_type( "zint:C16K",     typeof(BackendZint.C16k)     );
			glbarcode.Factory.register_type( "zint:C25M",     typeof(BackendZint.C25m)     );
			glbarcode.Factory.register_type( "zint:C25I",     typeof(BackendZint.C25i)     );
			glbarcode.Factory.register_type( "zint:C25DL",    typeof(BackendZint.C25dl)    );
			glbarcode.Factory.register_type( "zint:Code32",   typeof(BackendZint.Code32)   );
			glbarcode.Factory.register_type( "zint:Code39",   typeof(BackendZint.Code39)   );
			glbarcode.Factory.register_type( "zint:Code39E",  typeof(BackendZint.Code39e)  );
			glbarcode.Factory.register_type( "zint:Code49",   typeof(BackendZint.Code49)   );
			glbarcode.Factory.register_type( "zint:Code93",   typeof(BackendZint.Code93)   );
			glbarcode.Factory.register_type( "zint:Code128",  typeof(BackendZint.Code128)  );
			glbarcode.Factory.register_type( "zint:Code128B", typeof(BackendZint.Code128b) );
			glbarcode.Factory.register_type( "zint:DAFT",     typeof(BackendZint.Daft)     );
			glbarcode.Factory.register_type( "zint:DMTX",     typeof(BackendZint.Dmtx)     );
			glbarcode.Factory.register_type( "zint:DPL",      typeof(BackendZint.Dpl)      );
			glbarcode.Factory.register_type( "zint:DPI",      typeof(BackendZint.Dpi)      );
			glbarcode.Factory.register_type( "zint:KIX",      typeof(BackendZint.Kix)      );
			glbarcode.Factory.register_type( "zint:EAN",      typeof(BackendZint.Ean)      );
			glbarcode.Factory.register_type( "zint:GMTX",     typeof(BackendZint.Gmtx)     );
			glbarcode.Factory.register_type( "zint:GS1128",   typeof(BackendZint.Gs1128)   );
			glbarcode.Factory.register_type( "zint:RSS14",    typeof(BackendZint.Rss14)    );
			glbarcode.Factory.register_type( "zint:RSSLTD",   typeof(BackendZint.Rssltd)   );
			glbarcode.Factory.register_type( "zint:RSSEXP",   typeof(BackendZint.Rssexp)   );
			glbarcode.Factory.register_type( "zint:RSSS",     typeof(BackendZint.Rsss)     );
			glbarcode.Factory.register_type( "zint:RSSSO",    typeof(BackendZint.Rssso)    );
			glbarcode.Factory.register_type( "zint:RSSSE",    typeof(BackendZint.Rssse)    );
			glbarcode.Factory.register_type( "zint:HIBC128",  typeof(BackendZint.Hibc128)  );
			glbarcode.Factory.register_type( "zint:HIBC39",   typeof(BackendZint.Hibc39)   );
			glbarcode.Factory.register_type( "zint:HIBCDM",   typeof(BackendZint.Hibcdm)   );
			glbarcode.Factory.register_type( "zint:HIBCQR",   typeof(BackendZint.Hibcqr)   );
			glbarcode.Factory.register_type( "zint:HIBCPDF",  typeof(BackendZint.Hibcpdf)  );
			glbarcode.Factory.register_type( "zint:HIBCMPDF", typeof(BackendZint.Hibcmpdf) );
			glbarcode.Factory.register_type( "zint:HIBCAZ",   typeof(BackendZint.Hibcaz)   );
			glbarcode.Factory.register_type( "zint:I25",      typeof(BackendZint.I25)      );
			glbarcode.Factory.register_type( "zint:ISBN",     typeof(BackendZint.Isbn)     );
			glbarcode.Factory.register_type( "zint:ITF14",    typeof(BackendZint.Itf14)    );
			glbarcode.Factory.register_type( "zint:JAPAN",    typeof(BackendZint.Japan)    );
			glbarcode.Factory.register_type( "zint:KOREA",    typeof(BackendZint.Korea)    );
			glbarcode.Factory.register_type( "zint:LOGM",     typeof(BackendZint.Logm)     );
			glbarcode.Factory.register_type( "zint:MAXI",     typeof(BackendZint.Maxi)     );
			glbarcode.Factory.register_type( "zint:MPDF",     typeof(BackendZint.Mpdf)     );
			glbarcode.Factory.register_type( "zint:MQR",      typeof(BackendZint.Mqr)      );
			glbarcode.Factory.register_type( "zint:MSI",      typeof(BackendZint.Msi)      );
			glbarcode.Factory.register_type( "zint:NVE",      typeof(BackendZint.Nve)      );
			glbarcode.Factory.register_type( "zint:PDF",      typeof(BackendZint.Pdf)      );
			glbarcode.Factory.register_type( "zint:PDFT",     typeof(BackendZint.Pdft)     );
			glbarcode.Factory.register_type( "zint:PLAN",     typeof(BackendZint.Plan)     );
			glbarcode.Factory.register_type( "zint:POSTNET",  typeof(BackendZint.Postnet)  );
			glbarcode.Factory.register_type( "zint:PHARMA",   typeof(BackendZint.Pharma)   );
			glbarcode.Factory.register_type( "zint:PHARMA2",  typeof(BackendZint.Pharma2)  );
			glbarcode.Factory.register_type( "zint:PZN",      typeof(BackendZint.Pzn)      );
			glbarcode.Factory.register_type( "zint:QR",       typeof(BackendZint.Qr)       );
			glbarcode.Factory.register_type( "zint:RM4",      typeof(BackendZint.Rm4)      );
			glbarcode.Factory.register_type( "zint:TELE",     typeof(BackendZint.Tele)     );
			glbarcode.Factory.register_type( "zint:TELEX",    typeof(BackendZint.Telex)    );
			glbarcode.Factory.register_type( "zint:UPC-A",    typeof(BackendZint.UpcA)     );
			glbarcode.Factory.register_type( "zint:UPC-E",    typeof(BackendZint.UpcE)     );
			glbarcode.Factory.register_type( "zint:USPS",     typeof(BackendZint.Usps)     );
			glbarcode.Factory.register_type( "zint:PLS",      typeof(BackendZint.Pls)      );

			register_style( "zint:AUSP", "zint", _("Australia Post Standard"),
			                false, false, true, false, "12345678901234567890123", true, 23 );

			register_style( "zint:AUSRP", "zint", _("Australia Post Reply Paid"),
			                false, false, true, false, "12345678", true, 8 );

			register_style( "zint:AUSRT", "zint", _("Australia Post Route Code"),
			                false, false, true, false, "12345678", true, 8 );

			register_style( "zint:AUSRD", "zint", _("Australia Post Redirect"),
			                false, false, true, false, "12345678", true, 8 );

#if FIXME
			/* TODO: fix crash, zint-2.4.2 */
			register_style( "zint:AZTEC", "zint", _("Aztec Code"),
			                false, false, true, false, "1234567890", true, 10 );
#endif
          
			register_style( "zint:AZRUN", "zint", _("Aztec Rune"),
			                false, false, true, false, "255", true, 3 );

			register_style( "zint:CBR", "zint", _("Codabar"),
			                true, true, true, false, "ABCDABCDAB", true, 10 );

#if FIXME
			/* TODO: fix crash, zint-2.4.2 */
			register_style( "zint:Code1", "zint", _("Code One"), 
			                false, false, true, false, "0000000000", true, 10 );
#endif

			register_style( "zint:Code11", "zint", _("Code 11"),
			                true, true, true, false, "0000000000", true, 10 );
          
			register_style( "zint:C16K", "zint", _("Code 16K"),
			                false, false, true, false, "0000000000", true, 10 );
          
			register_style( "zint:C25M", "zint", _("Code 2 of 5 Matrix"), 
			                true, true, true, false, "0000000000", true, 10 );
          
			register_style( "zint:C25I", "zint", _("Code 2 of 5 IATA"), 
			                true, true, true, false, "0000000000", true, 10 );
          
			register_style( "zint:C25DL", "zint", _("Code 2 of 5 Data Logic"), 
			                true, true, true, false, "0000000000", true, 10 );

			register_style( "zint:Code32", "zint", _("Code 32 (Italian Pharmacode)"), 
			                true, true, true, false, "12345678", true, 8 );

			register_style( "zint:Code39", "zint", _("Code 39"),
			                true, true, false, false, "0000000000", true, 10 );
          
			register_style( "zint:Code39E", "zint", _("Code 39 Extended"), 
			                true, true, true, false, "0000000000", true, 10 );

			register_style( "zint:Code49", "zint", _("Code 49"),
			                false, false, true, false, "0000000000", true, 10 );

			register_style( "zint:Code93", "zint", _("Code 93"),
			                true, true, true, false, "0000000000", true, 10 );

			register_style( "zint:Code128", "zint", _("Code 128"),
			                true, true, true, false, "0000000000", true, 10 );
          
			register_style( "zint:Code128B", "zint", _("Code 128 (Mode C supression)"),
			                true, true, true, false, "0000000000", true, 10 );
          
			register_style( "zint:DAFT", "zint", _("DAFT Code"),
			                false, false, false, false, "DAFTDAFTDAFTDAFT", true, 16 );

			register_style( "zint:DMTX", "zint", _("Data Matrix"),
			                false, false, true, false, "0000000000", true, 10 );

			register_style( "zint:DPL", "zint", _("Deutsche Post Leitcode"),
			                true, true, true, false, "1234567890123", true, 13 );
          
			register_style( "zint:DPI", "zint", _("Deutsche Post Identcode"),
			                true, true, true, false, "12345678901", true, 11 );
          
			register_style( "zint:KIX", "zint", _("Dutch Post KIX Code"),
			                false, false, true, false, "0000000000", true, 10 );

			register_style( "zint:EAN", "zint", _("EAN"),
			                true, true, true, false, "1234567890123", false, 13 );

			register_style( "zint:GMTX", "zint", _("Grid Matrix"), 
			                false, false, true, false, "0000000000", true, 10 );

			register_style( "zint:GS1-128", "zint", _("GS1-128"),
			                true, true, true, false, "[01]12345678901234", false, 18 );

			register_style( "zint:RSS14", "zint", _("GS1 DataBar-14"),
			                true, true, true, false, "1234567890123", true, 13 );
          
			register_style( "zint:RSSLTD", "zint", "GS1 DataBar-14 Limited", 
			                true, true, true, false, "1234567890123", true, 13 );
          
			register_style( "zint:RSSEXP", "zint", "GS1 DataBar Extended", 
			                true, true, true, false, "[01]12345678901234", false, 18 );
          
			register_style( "zint:RSSS", "zint", _("GS1 DataBar-14 Stacked"),
			                false, false, true, false, "0000000000", true, 10 );

			register_style( "zint:RSSSO", "zint", _("GS1 DataBar-14 Stacked Omni."),
			                false, false, true, false, "0000000000", true, 10 );

			register_style( "zint:RSSSE", "zint", _("GS1 DataBar Extended Stacked"),
			                false, false, true, false, "[01]12345678901234", false, 18 );

			register_style( "zint:HIBC128", "zint", _("HIBC Code 128"),
			                true, true, true, false, "0000000000", true, 10 );

			register_style( "zint:HIBC39", "zint", _("HIBC Code 39"),
			                true, true, true, false, "0000000000", true, 10 );

			register_style( "zint:HIBCDM", "zint", _("HIBC Data Matrix"),
			                false, false, true, false, "0000000000", true, 10 );

			register_style( "zint:HIBCQR", "zint", _("HIBC QR Code"),
			                false, false, true, false, "0000000000", true, 10 );

			register_style( "zint:HIBCPDF", "zint", _("HIBC PDF417"),
			                false, false, true, false, "0000000000", true, 10 );

			register_style( "zint:HIBCMPDF", "zint", _("HIBC Micro PDF417"),
			                false, false, true, false, "0000000000", true, 10 );

			register_style( "zint:HIBCAZ", "zint", _("HIBC Aztec Code"),
			                true, true, true, false, "0000000000", true, 10 );

			register_style( "zint:I25", "zint", _("Interleaved 2 of 5"),
			                true, true, true, false, "0000000000", true, 10 );

			register_style( "zint:ISBN", "zint", _("ISBN"),
			                true, true, true, false, "123456789", false, 9 );

			register_style( "zint:ITF14", "zint", _("ITF-14"),
			                true, true, true, false, "0000000000", true, 10 );

			register_style( "zint:JAPAN", "zint", _("Japanese Postal"),
			                false, false, true, false, "0000000000", true, 10 );

			register_style( "zint:KOREA", "zint", _("Korean Postal"),
			                true, true, true, false, "123456", false, 6 );

			register_style( "zint:LOGM", "zint", _("LOGMARS"),
			                true, true, true, false, "0000000000", true, 10 );

			register_style( "zint:MAXI", "zint", _("Maxicode"),
			                false, false, false, false, "0000000000", true, 10 );

			register_style( "zint:MPDF", "zint", _("Micro PDF417"),
			                false, false, true, false, "0000000000", true, 10 );

			register_style( "zint:MQR", "zint", _("Micro QR Code"),
			                false, false, true, false, "0000000000", true, 10 );

			register_style( "zint:MSI", "zint", _("MSI Plessey"),
			                true, true, true, false, "0000000000", true, 10 );

			register_style( "zint:NVE", "zint", _("NVE-18"),
			                true, true, true, false, "12345678901234567", false, 17 );

			register_style( "zint:PDF", "zint", _("PDF417"),
			                false, false, true, false, "0000000000", true, 10 );

			register_style( "zint:PDFT", "zint", _("PDF417 Truncated"),
			                false, false, true, false, "0000000000", true, 10 );

			register_style( "zint:PLAN", "zint", _("PLANET"),
			                false, false, true, false, "0000000000", true, 10 );

			register_style( "zint:POSTNET", "zint", _("PostNet"),
			                true, true, true, false, "0000000000", true, 10 );

			register_style( "zint:PHARMA", "zint", _("Pharmacode"),
			                false, false, true, false, "123456", false, 6 );

			register_style( "zint:PHARMA2", "zint", _("Pharmacode 2-track"),
			                false, false, true, false, "12345678", false, 8 );

			register_style( "zint:PZN", "zint", _("Pharmazentral Nummer (PZN)"),
			                true, true, true, false, "123456", false, 6 );

			register_style( "zint:QR", "zint", _("QR Code"),
			                true, true, true, false, "0000000000", true, 10 );

			register_style( "zint:RM4", "zint", _("Royal Mail 4-State"),
			                false, false, true, false, "0000000000", true, 10 );

			register_style( "zint:TELE", "zint", _("Telepen"),
			                true, true, true, false, "0000000000", true, 10 );

			register_style( "zint:TELEX", "zint", _("Telepen Numeric"),
			                true, true, true, false, "0000000000", true, 10 );

			register_style( "zint:UPC-A", "zint", _("UPC-A"), 
			                true, true, true, false, "12345678901", false, 11 );
          
			register_style( "zint:UPC-E", "zint", _("UPC-E"), 
			                true, true, true, false, "1234567", false, 7 );
          
			register_style( "zint:USPS", "zint", _("USPS One Code"),
			                false, false, true, false, "12345678901234567890", true, 20 );

			register_style( "zint:PLS", "zint", _("UK Plessey"),
			                true, true, true, false, "0000000000", true, 10 );

#endif

#if HAVE_LIBIEC16022

			register_backend( "iec16022", _("IEC16022") );

			glbarcode.Factory.register_type( "iec16022:IEC16022", typeof(BackendIec16022.Datamatrix) );

			register_style( "iec16022:IEC16022", "iec16022", _("DataMatrix"),
			                false, false, true, false, "12345678", true, 8 );

#endif

#if HAVE_LIBQRENCODE

			register_backend( "qrencode", _("QREncode") );

			glbarcode.Factory.register_type( "qrencode:IEC18004", typeof(BackendQrencode.QRcode) );

			register_style( "qrencode:IEC18004", "qrencode", _("IEC18004 (QRCode)"),
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


		private static void register_backend( string id, string name )
		{
			backend_id_map.set( id, name );
			backend_name_map.set( name, id );
		}


		private static void register_style ( string id,
		                                     string backend_id,
		                                     string name,
		                                     bool   can_text,
		                                     bool   text_optional,
		                                     bool   can_checksum,
		                                     bool   checksum_optional,
		                                     string default_digits,
		                                     bool   can_freeform,
		                                     int    prefered_n )
		{
			BarcodeStyle style = new BarcodeStyle( id, backend_id, name,
			                                       can_text, text_optional, can_checksum, checksum_optional,
			                                       default_digits, can_freeform, prefered_n );

			id_map.set( id, style );
			name_map.set( "%s.%s".printf(backend_id, name), style ); /* Name may not be unique w/o backend */
		}


		public static string backend_id_to_name( string id )
		{
			if ( backend_id_map.has_key( id ) )
			{
				return backend_id_map.get( id );
			}
			else
			{
				return ""; /* Built-in backend. */
			}
		}


		public static string backend_name_to_id( string name )
		{
			if ( backend_name_map.has_key( name ) )
			{
				return backend_name_map.get( name );
			}
			else
			{
				return ""; /* Built-in backend. */
			}
		}


		public static List<string> get_backend_name_list()
		{
			List<string> list = null;

			foreach ( string backend_name in backend_id_map.values )
			{
				list.insert_sorted( backend_name, strcmp );
			}

			return list;
		}


		public static BarcodeStyle? lookup_style_from_id( string id )
		{
			if ( id_map.has_key( id ) )
			{
				return id_map.get( id );
			}
			else
			{
				return id_map.get( DEFAULT_ID );
			}
		}


		public static BarcodeStyle? lookup_style_from_name( string backend_id, string name )
		{
			string query_string = "%s.%s".printf(backend_id, name);

			if ( name_map.has_key( query_string ) )
			{
				return name_map.get( query_string );
			}
			else
			{
				return id_map.get( DEFAULT_ID );
			}
		}


		public static List<string> get_name_list( string backend_id )
		{
			List<string> list = null;

			foreach ( BarcodeStyle bc_style in id_map.values )
			{
				if ( bc_style.backend_id == backend_id )
				{
					list.insert_sorted( bc_style.name, strcmp );
				}
			}

			return list;
		}

	}

}




