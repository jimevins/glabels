
/*
 * Minimal VAPI file for LIBZINT Barcode library.
 */

#if HAVE_LIBZINT

namespace zint
{

	[CCode (cheader_filename = "zint.h", cname = "struct zint_symbol", unref_function = "ZBarcode_Delete")]
	public class Symbol
	{
		[CCode (cname = "ZBarcode_Create")]
		public Symbol();

		public int          symbology;
		public int          show_hrt;
		public string       errtxt;
		public Render      *rendered;

		[CCode (cname = "ZBarcode_Encode")]
		public int Encode( string input, int length );
		[CCode (cname = "ZBarcode_Render")]
		public int Render( float w, float h );
	}

	[CCode (cheader_filename = "zint.h", cname="struct zint_render")]
	public struct Render {
		float          width;
		float          height;
		RenderLine    *lines;
		RenderString  *strings;
		RenderRing    *rings;
		RenderHexagon *hexagons;
	}

	[CCode (cheader_filename = "zint.h", cname="struct zint_render_line")]
	public struct RenderLine {
		float          x;
		float          y;
		float          length;
		float          width;
		RenderLine    *next;
	}

	[CCode (cheader_filename = "zint.h", cname="struct zint_render_string")]
	public struct RenderString {
		float          x;
		float          y;
		float          fsize;
		float          width;
		int            length;
		string         text;
		RenderString  *next;
	}

	[CCode (cheader_filename = "zint.h", cname="struct zint_render_ring")]
	public struct RenderRing {
		float          x;
		float          y;
		float          radius;
		float          line_width;
		RenderRing    *next;
	}

	[CCode (cheader_filename = "zint.h", cname="struct zint_render_hexagon")]
	public struct RenderHexagon {
		float          x;
		float          y;
		RenderHexagon *next;
	}

	namespace Barcode
	{
		[CCode (cheader_filename = "zint.h", cname="BARCODE_AUSPOST")]
        public const int AUSPOST;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_AUSREPLY")]
        public const int AUSREPLY;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_AUSROUTE")]
        public const int AUSROUTE;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_AUSREDIRECT")]
        public const int AUSREDIRECT;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_AZTEC")]
        public const int AZTEC;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_AZRUNE")]
        public const int AZRUNE;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_CODABAR")]
        public const int CODABAR;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_CODEONE")]
        public const int CODEONE;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_CODE11")]
        public const int CODE11;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_CODE16K")]
        public const int CODE16K;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_C25MATRIX")]
        public const int C25MATRIX;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_C25IATA")]
        public const int C25IATA;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_C25LOGIC")]
        public const int C25LOGIC;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_CODE32")]
        public const int CODE32;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_CODE39")]
        public const int CODE39;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_EXCODE39")]
        public const int EXCODE39;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_CODE49")]
        public const int CODE49;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_CODE93")]
        public const int CODE93;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_CODE128")]
        public const int CODE128;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_CODE128B")]
        public const int CODE128B;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_DAFT")]
        public const int DAFT;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_DATAMATRIX")]
        public const int DATAMATRIX;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_DPLEIT")]
        public const int DPLEIT;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_DPIDENT")]
        public const int DPIDENT;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_KIX")]
        public const int KIX;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_EANX")]
        public const int EANX;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_HIBC_128")]
        public const int HIBC_128;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_HIBC_39")]
        public const int HIBC_39;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_HIBC_DM")]
        public const int HIBC_DM;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_HIBC_QR")]
        public const int HIBC_QR;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_HIBC_PDF")]
        public const int HIBC_PDF;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_HIBC_MICPDF")]
        public const int HIBC_MICPDF;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_HIBC_AZTEC")]
        public const int HIBC_AZTEC;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_C25INTER")]
        public const int C25INTER;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_ISBNX")]
        public const int ISBNX;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_ITF14")]
        public const int ITF14;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_GRIDMATRIX")]
        public const int GRIDMATRIX;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_EAN128")]
        public const int EAN128;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_LOGMARS")]
        public const int LOGMARS;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_RSS14")]
        public const int RSS14;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_RSS_LTD")]
        public const int RSS_LTD;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_RSS_EXP")]
        public const int RSS_EXP;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_RSS14STACK")]
        public const int RSS14STACK;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_RSS14STACK_OMNI")]
        public const int RSS14STACK_OMNI;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_RSS_EXPSTACK")]
        public const int RSS_EXPSTACK;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_PHARMA")]
        public const int PHARMA;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_PHARMA_TWO")]
        public const int PHARMA_TWO;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_PZN")]
        public const int PZN;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_TELEPEN")]
        public const int TELEPEN;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_TELEPEN_NUM")]
        public const int TELEPEN_NUM;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_JAPANPOST")]
        public const int JAPANPOST;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_KOREAPOST")]
        public const int KOREAPOST;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_MAXICODE")]
        public const int MAXICODE;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_MICROPDF417")]
        public const int MICROPDF417;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_PLESSEY")]
        public const int MSI_PLESSEY;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_MICROQR")]
        public const int MICROQR;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_NVE18")]
        public const int NVE18;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_PLANET")]
        public const int PLANET;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_POSTNET")]
        public const int POSTNET;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_PDF417")]
        public const int PDF417;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_PDF417TRUNC")]
        public const int PDF417TRUNC;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_QRCODE")]
        public const int QRCODE;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_RM4SCC")]
        public const int RM4SCC;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_UPCA")]
        public const int UPCA;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_UPCE")]
        public const int UPCE;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_ONECODE")]
        public const int ONECODE;
		[CCode (cheader_filename = "zint.h", cname="BARCODE_PLESSEY")]
        public const int PLESSEY;
	}

}

#endif
