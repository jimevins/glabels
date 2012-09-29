/*  zint.vala
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


#if HAVE_LIBZINT

using GLib;
using glbarcode.Constants;

namespace glabels
{

	namespace BackendZint
	{

		/**
		 * Concrete Barcode Classes
		 */
		public class AusP : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.AUSPOST;
				return "";
			}
		}

		public class AusRP : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.AUSREPLY;
				return "";
			}
		}

		public class AusRT : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.AUSROUTE;
				return "";
			}
		}

		public class AusRD : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.AUSREDIRECT;
				return "";
			}
		}

		public class Aztec : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.AZTEC;
				return "";
			}
		}

		public class Azrun : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.AZRUNE;
				return "";
			}
		}

		public class Cbr : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.CODABAR;
				return "";
			}
		}

		public class Code1 : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.CODEONE;
				return "";
			}
		}

		public class Code11 : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.CODE11;
				return "";
			}
		}

		public class C16k : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.CODE16K;
				return "";
			}
		}

		public class C25m : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.C25MATRIX;
				return "";
			}
		}

		public class C25i : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.C25IATA;
				return "";
			}
		}

		public class C25dl : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.C25LOGIC;
				return "";
			}
		}

		public class Code32 : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.CODE32;
				return "";
			}
		}

		public class Code39 : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.CODE39;
				return "";
			}
		}

		public class Code39e : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.EXCODE39;
				return "";
			}
		}

		public class Code49 : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.CODE49;
				return "";
			}
		}

		public class Code93 : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.CODE93;
				return "";
			}
		}

		public class Code128 : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.CODE128;
				return "";
			}
		}

		public class Code128b : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.CODE128B;
				return "";
			}
		}

		public class Daft : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.DAFT;
				return "";
			}
		}

		public class Dmtx : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.DATAMATRIX;
				return "";
			}
		}

		public class Dpl : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.DPLEIT;
				return "";
			}
		}

		public class Dpi : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.DPIDENT;
				return "";
			}
		}

		public class Kix : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.KIX;
				return "";
			}
		}

		public class Ean : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.EANX;
				return "";
			}
		}

		public class Hibc128 : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.HIBC_128;
				return "";
			}
		}

		public class Hibc39 : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.HIBC_39;
				return "";
			}
		}

		public class Hibcdm : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.HIBC_DM;
				return "";
			}
		}

		public class Hibcqr : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.HIBC_QR;
				return "";
			}
		}

		public class Hibcpdf : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.HIBC_PDF;
				return "";
			}
		}

		public class Hibcmpdf : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.HIBC_MICPDF;
				return "";
			}
		}

		public class Hibcaz : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.HIBC_AZTEC;
				return "";
			}
		}

		public class I25 : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.C25INTER;
				return "";
			}
		}

		public class Isbn : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.ISBNX;
				return "";
			}
		}

		public class Itf14 : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.ITF14;
				return "";
			}
		}

		public class Gmtx : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.GRIDMATRIX;
				return "";
			}
		}

		public class Gs1128 : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.EAN128;
				return "";
			}
		}

		public class Logm : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.LOGMARS;
				return "";
			}
		}

		public class Rss14 : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.RSS14;
				return "";
			}
		}

		public class Rssltd : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.RSS_LTD;
				return "";
			}
		}

		public class Rssexp : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.RSS_EXP;
				return "";
			}
		}

		public class Rsss : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.RSS14STACK;
				return "";
			}
		}

		public class Rssso : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.RSS14STACK_OMNI;
				return "";
			}
		}

		public class Rssse : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.RSS_EXPSTACK;
				return "";
			}
		}

		public class Pharma : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.PHARMA;
				return "";
			}
		}

		public class Pharma2 : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.PHARMA_TWO;
				return "";
			}
		}

		public class Pzn : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.PZN;
				return "";
			}
		}

		public class Tele : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.TELEPEN;
				return "";
			}
		}

		public class Telex : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.TELEPEN_NUM;
				return "";
			}
		}

		public class Japan : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.JAPANPOST;
				return "";
			}
		}

		public class Korea : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.KOREAPOST;
				return "";
			}
		}

		public class Maxi : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.MAXICODE;
				return "";
			}
		}

		public class Mpdf : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.MICROPDF417;
				return "";
			}
		}

		public class Msi : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.MSI_PLESSEY;
				return "";
			}
		}

		public class Mqr : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.MICROQR;
				return "";
			}
		}

		public class Nve : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.NVE18;
				return "";
			}
		}

		public class Plan : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.PLANET;
				return "";
			}
		}

		public class Postnet : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.POSTNET;
				return "";
			}
		}

		public class Pdf : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.PDF417;
				return "";
			}
		}

		public class Pdft : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.PDF417TRUNC;
				return "";
			}
		}

		public class Qr : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.QRCODE;
				return "";
			}
		}

		public class Rm4 : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.RM4SCC;
				return "";
			}
		}

		public class UpcA : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.UPCA;
				return "";
			}
		}

		public class UpcE : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.UPCE;
				return "";
			}
		}

		public class Usps : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.ONECODE;
				return "";
			}
		}

		public class Pls : BarcodeBase
		{
			protected override string encode( string data )
			{
				my_symbology = zint.Barcode.PLESSEY;
				return "";
			}
		}


		/**
		 * Base class for Zint backend barcodes
		 */
		public abstract class BarcodeBase : glbarcode.Barcode
		{

			private const int W_DEFAULT = 144;
			private const int H_DEFAULT =  72;


			protected int my_symbology;


			/**
			 * Zint Barcode data validation method
			 */
			protected override bool validate( string data )
			{
				if (data.length == 0)
				{
					return false;
				}

				return true;
			}


			/**
			 * Zint Barcode vectorization method
			 */
			protected override void vectorize( string coded_data, string data, string text )
			{
				/*
				 * First encode using Zint barcode library.
				 */
				if ( w == 0 )
				{
					w = W_DEFAULT;
				}
				if ( h == 0 )
				{
					h = H_DEFAULT;
				}

				zint.Symbol symbol = new zint.Symbol();

				symbol.symbology = my_symbology;

				if ( symbol.Encode( data, 0 ) != 0 )
				{
					debug( "Zint::ZBarcode_Encode:  %s\n", symbol.errtxt );
					is_data_valid = false;
					return;
				}

				symbol.show_hrt = text_flag ? 1 : 0;

				if ( symbol.Render( (float)w, (float)h ) == 0 )
				{
					message( "Zint::ZBarcode_Render:  %s\n", symbol.errtxt );
					is_data_valid = false;
					return;
				}


				/*
				 * Now do the actual vectorization.
				 */
				zint.Render *render = symbol.rendered;

				for ( zint.RenderLine *zline = render->lines; zline != null; zline = zline->next )
				{
					add_box( zline->x, zline->y, zline->width, zline->length );
				}

				for ( zint.RenderRing *zring = render->rings; zring != null; zring = zring->next )
				{
					add_ring( zring->x, zring->y, zring->radius, zring->line_width );
				}

				for ( zint.RenderHexagon *zhexagon = render->hexagons; zhexagon != null; zhexagon = zhexagon->next )
				{
					add_hexagon( zhexagon->x, zhexagon->y, 2.89 );
				}

				if(text_flag)
				{
					for ( zint.RenderString *zstring = render->strings; zstring != null; zstring = zstring->next )
					{
						add_string( zstring->x, zstring->y, zstring->fsize, zstring->text );
					}
				}

			}


		}

	}

}

#endif
