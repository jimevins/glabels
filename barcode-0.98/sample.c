#include <stdio.h>
#include <stdlib.h>

#include "barcode.h"

int main(int argc, char **argv)
{
    int ps = 1, pcl = 0, oflags;
    if (argc == 2 && !strcmp(argv[1],"-P")) {
	ps = 0; pcl = 1; argc=1;
    }
    if (argc>2) {
	fprintf(stderr, "%s: use \"%s\" for postscript or \"%s -P\" for PCL\n",
		argv[0], argv[0], argv[0]);
	exit(1);
    }
    if (pcl) {
	oflags = BARCODE_OUT_PCL;
    } else {
	oflags = BARCODE_OUT_PS | BARCODE_OUT_NOHEADERS;
	printf("%%!PS-Adobe-2.0\n");
	printf("%%%%Creator: barcode sample program\n");
	printf("%%%%EndComments\n");
	printf("%%%%EndProlog\n\n");
	printf("%%%%Page: 1 1\n\n");
    }
    /* Print a few barcodes in several places in the page */

    /* default size, bottom left */
    Barcode_Encode_and_Print("800894002700",stdout, 0, 0, 40, 40, 
                      BARCODE_EAN | oflags);

    /* smaller */
    Barcode_Encode_and_Print("800894002700",stdout, 70, 50, 160, 55, 
                      BARCODE_EAN | oflags);

    /* smallest */
    Barcode_Encode_and_Print("800894002700",stdout, 40, 30, 270, 70, 
                      BARCODE_EAN | oflags);

    /* A bigger all-0 */
    Barcode_Encode_and_Print("000000000000",stdout, 170, 0, 40, 160, 
                      BARCODE_EAN | oflags);

    /* Still bigger all-0 (but UPC, this time) */
    Barcode_Encode_and_Print("00000000000",stdout, 250, 0, 270, 160, 
                      BARCODE_UPC | oflags);

    /* A few code-39 ones */
    Barcode_Encode_and_Print("silly code",stdout, 0, 0, 40, 320, 
                      BARCODE_39 | oflags);
    Barcode_Encode_and_Print("SAMPLE CODES",stdout, 100, 30, 400, 80, 
                      BARCODE_39 | oflags);

    /* ISBN with add-5 */
    Barcode_Encode_and_Print("1-56592-292-1 90000",stdout, 0, 0, 40, 430, 
                      BARCODE_ISBN | oflags);

    /* UPC with add-2 */
    Barcode_Encode_and_Print("07447084452 07",stdout, 0, 0, 300, 410, 
                      BARCODE_UPC | oflags);

    /* code 128-C */
    Barcode_Encode_and_Print("12345678900123456789",stdout, 0, 0, 40, 530, 
                      BARCODE_128C | oflags);

    /* and my data as code-128B autodetected */
    Barcode_Encode_and_Print("RBNLSN68T11E897W",stdout, 0, 60, 240, 510, 
                oflags);
    /* same as code-39, forced */
    Barcode_Encode_and_Print("RBNLSN68T11E897W",stdout, 0, 60, 240, 590, 
                BARCODE_NO_CHECKSUM | BARCODE_39 | oflags);

    /* one interleaved 2 of 5 */
    Barcode_Encode_and_Print("0123456789",stdout, 0, 0, 40, 620, 
                BARCODE_I25 | oflags);

    /* upc-e and ean-8 (autotected based on code size) */
    Barcode_Encode_and_Print("012345",stdout, 0, 0, 50, 720, oflags);
    Barcode_Encode_and_Print("0123456",stdout, 0, 0, 160, 720, oflags);


    
    if (pcl) {
	printf("\f");
    } else {
	printf("\nshowpage\n");
	printf("%%%%Trailer\n\n");
    }
    return 0;
}







