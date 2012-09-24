/*  gnu_barcode.vala
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


#if HAVE_LIBBARCODE

using GLib;
using glbarcode.Constants;

namespace glabels
{

	namespace BackendGnuBarcode
	{

		/**
		 * EAN Barcode (Any)
		 */
		public class Ean : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return (    ( is_numeric_length_valid(  data,  7,  8 ) )
				         || ( is_numeric_length_valid(  data, 12, 13 ) )
				         || ( is_numeric_length1_valid( data,  7,  8 ) && is_numeric_length2_valid( data, 2, 2 ) )
				         || ( is_numeric_length1_valid( data,  7,  8 ) && is_numeric_length2_valid( data, 5, 5 ) )
				         || ( is_numeric_length1_valid( data, 12, 13 ) && is_numeric_length2_valid( data, 2, 2 ) )
				         || ( is_numeric_length1_valid( data, 12, 13 ) && is_numeric_length2_valid( data, 5, 5 ) )
					);
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.EAN;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * EAN-8 Barcode
		 */
		public class Ean8 : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_numeric_length_valid( data, 7, 8 );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.EAN;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * EAN-8+2 Barcode
		 */
		public class Ean8_2 : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_numeric_length1_valid( data, 7, 8 ) && is_numeric_length2_valid( data, 2, 2 );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.EAN;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * EAN-8+5 Barcode
		 */
		public class Ean8_5 : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_numeric_length1_valid( data, 7, 8 ) && is_numeric_length2_valid( data, 5, 5 );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.EAN;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * EAN-13 Barcode
		 */
		public class Ean13 : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_numeric_length_valid( data, 12, 13 );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.EAN;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * EAN-13+2 Barcode
		 */
		public class Ean13_2 : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_numeric_length1_valid( data, 12, 13 ) && is_numeric_length2_valid( data, 2, 2 );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.EAN;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * EAN-13+5 Barcode
		 */
		public class Ean13_5 : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_numeric_length1_valid( data, 12, 13 ) && is_numeric_length2_valid( data, 5, 5 );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.EAN;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * UPC Barcode (Any)
		 */
		public class Upc : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return (    ( is_numeric_length_valid(  data,  6,  8 ) )
				         || ( is_numeric_length_valid(  data, 11, 12 ) )
				         || ( is_numeric_length1_valid( data,  6,  8 ) && is_numeric_length2_valid( data, 2, 2 ) )
				         || ( is_numeric_length1_valid( data,  6,  8 ) && is_numeric_length2_valid( data, 5, 5 ) )
				         || ( is_numeric_length1_valid( data, 11, 12 ) && is_numeric_length2_valid( data, 2, 2 ) )
				         || ( is_numeric_length1_valid( data, 11, 12 ) && is_numeric_length2_valid( data, 5, 5 ) )
					);
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.UPC;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * UPC-A Barcode
		 */
		public class UpcA : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_numeric_length_valid( data, 11, 12 );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.UPC;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * UPC-A+2 Barcode
		 */
		public class UpcA_2 : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_numeric_length1_valid( data, 11, 12 ) && is_numeric_length2_valid( data, 2, 2 );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.UPC;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * UPC-A+5 Barcode
		 */
		public class UpcA_5 : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_numeric_length1_valid( data, 11, 12 ) && is_numeric_length2_valid( data, 5, 5 );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.UPC;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * UPC-E Barcode
		 */
		public class UpcE : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_numeric_length_valid( data, 6, 8 );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.UPC;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * UPC-E+2 Barcode
		 */
		public class UpcE_2 : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_numeric_length1_valid( data, 6, 8 ) && is_numeric_length2_valid( data, 2, 2 );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.UPC;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * UPC-E+5 Barcode
		 */
		public class UpcE_5 : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_numeric_length1_valid( data, 6, 8 ) && is_numeric_length2_valid( data, 5, 5 );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.UPC;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * ISBN Barcode
		 */
		public class Isbn : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_numeric_length_valid(  data,  9,  10 );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.ISBN;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * ISBN+5 Barcode
		 */
		public class Isbn_5 : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_numeric_length1_valid( data, 9, 10 ) && is_numeric_length2_valid( data, 5, 5 );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.ISBN;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * Code39 Barcode
		 */
		public class Code39 : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_ascii( data );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.CODE39;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * Code128 Barcode
		 */
		public class Code128 : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_ascii( data );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.CODE128;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * Code128C Barcode
		 */
		public class Code128C : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_ascii( data );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.CODE128C;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * Code128B Barcode
		 */
		public class Code128B : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_ascii( data );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.CODE128B;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * I25 Barcode
		 */
		public class I25 : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_ascii( data );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.I25;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * CBR Barcode
		 */
		public class Cbr : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_ascii( data );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.CBR;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * MSI Barcode
		 */
		public class Msi : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_ascii( data );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.MSI;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * PLS Barcode
		 */
		public class Pls : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_ascii( data );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.PLS;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * Code93 Barcode
		 */
		public class Code93 : BarcodeBase
		{
			protected override bool validate( string data )
			{
				return is_ascii( data );
			}

			protected override string encode( string data )
			{
				flags = GNU.Barcode.Flags.CODE93;

				return ""; /* Actual encoding done in vectorize. */
			}
		}


		/**
		 * Base class for GNU-Barcode backend barcodes
		 */
		public abstract class BarcodeBase : glbarcode.Barcode
		{

			protected int flags;


			/**
			 * GNU Barcode validate if valid ASCII
			 */
			protected bool is_ascii( string data )
			{
				for ( int i = 0; i < data.length; i++ )
				{
					if ( (data[i] & 0x80) != 0 )
					{
						return false;
					}
				}

				return true;
			}


			/**
			 * GNU Barcode validate length of numeric data string
			 */
			protected bool is_numeric_length_valid( string data, int n_min, int n_max )
			{
				int n = 0;

				for ( int i = 0; i < data.length; i++ )
				{
					if ( (data[i] & 0x80) == 0 )
					{
						n++;
					}
					else
					{
						return false;
					}
				}

				return (n >= n_min) && (n <= n_max);
			}


			/**
			 * GNU Barcode validate length of 1st space separated numeric string
			 */
			protected bool is_numeric_length1_valid( string data, int n_min, int n_max )
			{
				int n = 0;

				for ( int i = 0; !data[i].isspace() && (i < data.length); i++ )
				{
					if ( data[i].isdigit() )
					{
						n++;
					}
					else if ( !data[i].isspace() )
					{
						return false;
					}
				}

				return (n >= n_min) && (n <= n_max);
			}


			/**
			 * GNU Barcode validate length of 2nd space separated numeric string
			 */
			protected bool is_numeric_length2_valid( string data, int n_min, int n_max )
			{
				int i;
				int n = 0;

				for ( i = 0; !data[i].isspace() && (i < data.length); i++ )
				{
					/* Skip over 1st string */
				}

				for ( i++ /* skip space */ ; i < data.length; i++ )
				{
					if ( data[i].isdigit() )
					{
						n++;
					}
					else
					{
						return false;
					}
				}

				return (n >= n_min) && (n <= n_max);
			}


			/**
			 * GNU Barcode vectorization method
			 */
			protected override void vectorize( string coded_data, string data, string text )
			{
				/*
				 * First encode using GNU Barcode library.
				 */
				GNU.Barcode.Item bci = new GNU.Barcode.Item( data );

				bci.scalef = 0;
				bci.width  = (int)w;
				bci.height = (int)h;

				bci.flags = flags;
				if ( !text_flag )
				{
					bci.flags |= GNU.Barcode.Flags.NO_ASCII;
				}
				if ( !checksum_flag )
				{
					bci.flags |= GNU.Barcode.Flags.NO_CHECKSUM;
				}

				bci.Encode( flags );

				if ( (bci.partial == null) || (bci.textinfo == null) )
				{
					is_data_valid = false;
					return;
				}


				/*
				 * Now do the actual vectorization.
				 *
				 * This code is based on the postscript renderer (ps.c) from the GNU barcode library:
				 *
				 *     Copyright (C) 1999 Alessaandro Rubini (rubini@gnu.org)
				 *     Copyright (C) 1999 Prosa Srl. (prosa@prosa.it)
				 */
				if (bci.width > (2*bci.margin))
				{
					bci.width -= 2*bci.margin;
				}
				if (bci.height > (2*bci.margin))
				{
					bci.height -= 2*bci.margin;
				}

				/* First calculate barlen */
				int barlen = bci.partial[0].digit_value();
				for ( int i = 1; i < bci.partial.length; i++ )
				{
					if ( bci.partial[i].isdigit() )
					{
						barlen += bci.partial[i].digit_value();
					}
					else
					{
						if ( (bci.partial[i] != '+') && (bci.partial[i] != '-') )
						{
							barlen += bci.partial[i] - 'a' + 1;
						}
					}
				}

				/* The scale factor depends on bar length */
				double scalef = 1;
				if ( bci.scalef == 0 )
				{
					if ( bci.width == 0 )
					{
						bci.width = barlen; /* default */
					}
					scalef = bci.scalef = (double)bci.width / (double)barlen;
					if ( scalef < 0.5 )
					{
						scalef = 0.5;
					}
				}

				/* The width defaults to "just enough" */
				bci.width = (int)( barlen * scalef + 1 );

				/* But it can be too small, in this case enlarge and center the area */
				if ( bci.width < (int)(barlen * scalef) )
				{
					int wid = (int)( barlen * scalef + 1);
					bci.xoff -= (wid - bci.width)/2 ;
					bci.width = wid;
					/* Can't extend too far on the left */
					if (bci.xoff < 0)
					{
						bci.width += -bci.xoff;
						bci.xoff = 0;
					}
				}

				/* The height defaults to 80 points (rescaled) */
				if ( bci.height == 0 )
				{
					bci.height = (int)( 80 * scalef );
				}

				/* If too small (5 + text), reduce the scale factor and center */
				int i = 5 + 10 * ( ((bci.flags & GNU.Barcode.Flags.NO_ASCII)==0) ? 1 : 0 );
				if ( bci.height < (int)(i * scalef) )
				{
					bci.height = (int)( i * scalef );
				}

				/* Now traverse the code string and create a list of lines */
				char mode = '-'; /* text below bars */
				double x = bci.margin + bci.partial[0].digit_value() * scalef;
				i = 1;
				for ( int ip = 1; ip < bci.partial.length; ip++, i++)
				{
					/* special cases: '+' and '-' */
					if ( bci.partial[ip] == '+' || bci.partial[ip] == '-' )
					{
						mode = bci.partial[ip];        /* don't count it */
						i++;
						continue;
					}
					/* j is the width of this bar/space */
					int j;
					if ( bci.partial[ip].isdigit() )
					{
						j = bci.partial[ip].digit_value();
					}
					else
					{
						j = bci.partial[ip] - 'a' + 1;
					}
					if ( (i % 2) != 0 )
					{
						/* bar */
						double x0 = x;
						double y0 = bci.margin;
						double yr = bci.height;
						if ( (bci.flags & GNU.Barcode.Flags.NO_ASCII) == 0 )
						{
							/* leave space for text */
							if (mode == '-')
							{
								/* text below bars: 10 or 5 points */
								yr -= (bci.partial[ip].isdigit() ? 10 : 5) * scalef;
							}
							else
							{
								/* '+' */
								/* above bars: 10 or 0 from bottom,
								   and 10 from top */
								y0 += 10 * scalef;
								yr -= (bci.partial[ip].isdigit() ? 20 : 10) * scalef;
							}
						}
						add_box( x0, y0, (j * scalef) - glbarcode.Constants.INK_BLEED, yr );
					}
					x += j * scalef;

				}

				/* Now the text */
				mode = '-';                /* reinstantiate default */
				if ( (bci.flags & GNU.Barcode.Flags.NO_ASCII) == 0 )
				{
					for ( i = 0; i < bci.textinfo.length; i++ )
					{
						if ( bci.textinfo[i] == ' ' )
						{
							continue;
						}
						if ( (bci.textinfo[i] == '+') || (bci.textinfo[i] == '-') )
						{
							mode = bci.textinfo[i];
							continue;
						}
						double f1, f2;
						char   c;
						if ( bci.textinfo.substring(i).scanf("%lf:%lf:%c", out f1, out f2, out c) != 3 )
						{
							message( "impossible data: %s", bci.textinfo.substring(i) );
							continue;
						}
						double x0 = f1 * scalef + bci.margin + (f2 * scalef)/4;
						double y0;
						if (mode == '-')
						{
							y0 = bci.margin + bci.height - 8 * scalef;
						}
						else
						{
							y0 = bci.margin;
						}
						add_string( x0, y0, (f2 * scalef), c.to_string() );

						/* skip past the substring we just read. */
						while ( (bci.textinfo[i] != ' ') && (bci.textinfo[i] != 0) )
						{
							i++;
						}
					}
				}

				/* Fill in other info */
				w = bci.width  + 2.0*bci.margin;
				h = bci.height + 2.0*bci.margin;

			}


		}

	}

}

#endif
