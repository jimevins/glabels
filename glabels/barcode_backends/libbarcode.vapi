
/*
 * Minimal VAPI file for GNU Barcode library.
 */

#if HAVE_LIBBARCODE

namespace GNU
{

	namespace Barcode
	{

		[CCode (cheader_filename = "barcode.h", cname = "struct Barcode_Item", unref_function = "Barcode_Delete")]
		public class Item
		{
			[CCode (cname = "Barcode_Create")]
			public Item( string text );

			public int         flags;
			public weak string ascii;
			public weak string partial;
			public weak string textinfo;
			public weak string encoding;
			public int         width;
			public int         height;
			public int         xoff;
			public int         yoff;
			public int         margin;
			public double      scalef;
			public int         error;

			[CCode (cheader_filename = "barcode.h", cname = "Barcode_Encode")]
			public int Encode( int flags );
		}

		namespace Flags
		{
			[CCode (cheader_filename = "barcode.h", cname = "BARCODE_DEFAULT_FLAGS")]
			public const int DEFAULT_FLAGS;
			[CCode (cheader_filename = "barcode.h", cname = "BARCODE_ENCODING_MASK")]
			public const int ENCODING_MASK;
			[CCode (cheader_filename = "barcode.h", cname = "BARCODE_NO_ASCII")]
			public const int NO_ASCII;
			[CCode (cheader_filename = "barcode.h", cname = "BARCODE_NO_CHECKSUM")]
			public const int NO_CHECKSUM;
			[CCode (cheader_filename = "barcode.h", cname = "BARCODE_EAN")]
			public const int EAN;
			[CCode (cheader_filename = "barcode.h", cname = "BARCODE_UPC")]
			public const int UPC;
			[CCode (cheader_filename = "barcode.h", cname = "BARCODE_ISBN")]
			public const int ISBN;
			[CCode (cheader_filename = "barcode.h", cname = "BARCODE_39")]
			public const int CODE39;
			[CCode (cheader_filename = "barcode.h", cname = "BARCODE_128")]
			public const int CODE128;
			[CCode (cheader_filename = "barcode.h", cname = "BARCODE_128C")]
			public const int CODE128C;
			[CCode (cheader_filename = "barcode.h", cname = "BARCODE_128B")]
			public const int CODE128B;
			[CCode (cheader_filename = "barcode.h", cname = "BARCODE_I25")]
			public const int I25;
			[CCode (cheader_filename = "barcode.h", cname = "BARCODE_128RAW")]
			public const int CODE128RAW;
			[CCode (cheader_filename = "barcode.h", cname = "BARCODE_CBR")]
			public const int CBR;
			[CCode (cheader_filename = "barcode.h", cname = "BARCODE_MSI")]
			public const int MSI;
			[CCode (cheader_filename = "barcode.h", cname = "BARCODE_PLS")]
			public const int PLS;
			[CCode (cheader_filename = "barcode.h", cname = "BARCODE_93")]
			public const int CODE93;
		}

	}

}

#endif
