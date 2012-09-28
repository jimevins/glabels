
/*
 * Minimal VAPI file for LIBIEC16022 Barcode library.
 */

#if HAVE_LIBIEC16022

namespace iec16022
{

	[CCode (cheader_filename = "iec16022ecc200.h", cname = "iec16022ecc200", array_length = false)]
	public char[] ecc200( ref int iw, ref int ih, char** encoding,
	                      int len, string data,
	                      int* lenp, int* maxp, int* eccp );

	[CCode (cheader_filename = "iec16022ecc200.h", cname = "MAXBARCODE")]
	public const int MAXBARCODE;

}

#endif
