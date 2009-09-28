/** 
 *
 * IEC16022 bar code generation
 * Adrian Kennard, Andrews & Arnold Ltd
 * with help from Cliff Hones on the RS coding
 *
 * (c) 2004 Adrian Kennard, Andrews & Arnold Ltd
 * (c) 2006 Stefan Schmidt <stefan@datenfreihafen.org>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#define IEC16022_VERSION "0.2"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <popt.h>
#include <malloc.h>
#include "image.h"
#include "iec16022ecc200.h"

 // simple checked response malloc
void *safemalloc(int n)
{
	void *p = malloc(n);
	if (!p) {
		fprintf(stderr, "Malloc(%d) failed\n", n);
		exit(1);
	}
	return p;
}

// hex dump - bottom left pixel first
void dumphex(unsigned char *grid, int W, int H, unsigned char p)
{
	int c = 0, y;
	for (y = 0; y < H; y++) {
		int v = 0, x, b = 128;
		for (x = 0; x < W; x++) {
			if (grid[y * W + x])
				v |= b;
			b >>= 1;
			if (!b) {
				printf("%02X", v ^ p);
				v = 0;
				b = 128;
				c++;
			}
		}
		if (b != 128) {
			printf("%02X", v ^ p);
			c++;
		}
		printf(" ");
		c++;
		if (c >= 80) {
			printf("\n");
			c = 0;
		}
	}
	if (c)
		printf("\n");
}

int main(int argc, const char *argv[])
{
	char c;
	int W = 0, H = 0;
	int ecc = 0;
	int barcodelen = 0;
	char *encoding = 0;
	char *outfile = 0;
	char *infile = 0;
	char *barcode = 0;
	char *format = "Text";
	char *size = 0;
	char *eccstr = 0;
	int len = 0, maxlen = 0, ecclen = 0;
	unsigned char *grid = 0;
	poptContext optCon;	// context for parsing command-line options
	const struct poptOption optionsTable[] = {
		{
		 "size", 's', POPT_ARG_STRING, &size, 0, "Size", "WxH"},
		{
		 "barcode", 'c', POPT_ARG_STRING, &barcode, 0, "Barcode",
		 "text"},
		{
		 "ecc", 0, POPT_ARG_STRING, &eccstr, 0, "ECC",
		 "000/050/080/100/140/200"},
		{
		 "infile", 'i', POPT_ARG_STRING, &infile, 0, "Barcode file",
		 "filename"},
		{
		 "outfile", 'o', POPT_ARG_STRING, &outfile, 0,
		 "Output filename",
		 "filename"},
		{
		 "encoding", 'e', POPT_ARG_STRING, &encoding, 0,
		 "Encoding template",
		 "[CTXEAB]* for ecc200 or 11/27/41/37/128/256"},
		{
		 "format", 'f', POPT_ARGFLAG_SHOW_DEFAULT | POPT_ARG_STRING,
		 &format, 0,
		 "Output format", "Text/EPS/PNG/Bin/Hex/Stamp"},
		POPT_AUTOHELP {
			       NULL, 0, 0, NULL, 0}
	};
	optCon = poptGetContext(NULL, argc, argv, optionsTable, 0);
	poptSetOtherOptionHelp(optCon, "[barcode]");
	if ((c = poptGetNextOpt(optCon)) < -1) {
		/* an error occurred during option processing */
		fprintf(stderr, "%s: %s\n", poptBadOption(optCon,
							  POPT_BADOPTION_NOALIAS),
			poptStrerror(c));
		return 1;
	}

	if (poptPeekArg(optCon) && !barcode && !infile)
		barcode = (char *)poptGetArg(optCon);
	if (poptPeekArg(optCon) || !barcode && !infile || barcode && infile) {
		fprintf(stderr, "Version: %s\n", IEC16022_VERSION);
		poptPrintUsage(optCon, stderr, 0);
		return -1;
	}
	if (outfile && !freopen(outfile, "w", stdout)) {
		perror(outfile);
		return 1;
	}

	if (infile) {		// read from file
		FILE *f = fopen(infile, "rb");
		barcode = safemalloc(4001);
		if (!f) {
			perror(infile);
			return 1;
		}
		barcodelen = fread(barcode, 1, 4000, f);
		if (barcodelen < 0) {
			perror(infile);
			return 1;
		}
		barcode[barcodelen] = 0;	// null terminate anyway
		close(f);
	} else
		barcodelen = strlen(barcode);
	// check parameters
	if (size) {
		char *x = strchr(size, 'x');
		W = atoi(size);
		if (x)
			H = atoi(x + 1);
		if (!H)
			W = H;
	}
	if (eccstr)
		ecc = atoi(eccstr);
	if (W & 1) {		// odd size
		if (W != H || W < 9 || W > 49) {
			fprintf(stderr, "Invalid size %dx%d\n", W, H);
			return 1;
		}
		if (!eccstr) {
			if (W >= 17)
				ecc = 140;
			else if (W >= 13)
				ecc = 100;
			else if (W >= 11)
				ecc = 80;
			else
				ecc = 0;
		}
		if (ecc && ecc != 50 && ecc != 80 && ecc != 100 && ecc != 140 ||
		    ecc == 50 && W < 11 || ecc == 80 && W < 13 || ecc == 100
		    && W < 13 || ecc == 140 && W < 17) {
			fprintf(stderr, "ECC%03d invalid for %dx%d\n", ecc, W,
				H);
			return 1;
		}

	} else if (W) {		// even size
		if (W < H) {
			int t = W;
			W = H;
			H = t;
		}
		if (!eccstr)
			ecc = 200;
		if (ecc != 200) {
			fprintf(stderr, "ECC%03d invalid for %dx%d\n", ecc, W,
				H);
			return 1;
		}
	}

	else {			// auto size
		if (!eccstr)
			// default is even sizes only unless explicit ecc set to force odd
			// sizes
			ecc = 200;
	}

	if (tolower(*format) == 's') {	// special stamp format checks & defaults
		if (!W)
			W = H = 32;
		if (ecc != 200 || W != 32 || H != 32)
			fprintf(stderr, "Stamps must be 32x32\n");
		if (encoding)
			fprintf(stderr, "Stamps should use auto encoding\n");
		else {
			int n;
			for (n = 0; n < barcodelen && (barcode[n] == ' ' ||
						       isdigit(barcode[n])
						       || isupper(barcode[n]));
			     n++) ;
			if (n < barcodelen)
				fprintf(stderr,
					"Has invalid characters for a stamp\n");
			else {
				// Generate simplistic encoding rules as used by the windows app
				// TBA - does not always match the windows app...
				n = 0;
				encoding = safemalloc(barcodelen + 1);
				while (n < barcodelen) {
					// ASCII
					while (1) {
						if (n == barcodelen
						    || n + 3 <= barcodelen
						    && (!isdigit(barcode[n])
							||
							!isdigit(barcode
								 [n + 1])))
							break;
						encoding[n++] = 'A';
						if (n < barcodelen
						    && isdigit(barcode[n - 1])
						    && isdigit(barcode[n]))
							encoding[n++] = 'A';
					}
					// C40
					while (1) {
						int r = 0;
						while (n + r < barcodelen
						       &&
						       isdigit(barcode[n + r]))
							r++;
						if (n + 3 > barcodelen
						    || r >= 6)
							break;
						encoding[n++] = 'C';
						encoding[n++] = 'C';
						encoding[n++] = 'C';
					}
				}
				encoding[n] = 0;
				//fprintf (stderr, "%s\n%s\n", barcode, encoding);
			}
		}
	}
	// processing stamps
	if ((W & 1) || ecc < 200) {	// odd sizes
		fprintf(stderr, "Not done odd sizes yet, sorry\n");
	} else {		// even sizes
		grid =
		    iec16022ecc200(&W, &H, &encoding, barcodelen, barcode, &len,
				   &maxlen, &ecclen);
	}

	// output
	if (!grid || !W) {
		fprintf(stderr, "No barcode produced\n");
		return 1;
	}
	switch (tolower(*format)) {
	case 'i':		// info
		printf("Size    : %dx%d\n", W, H);
		printf("Encoded : %d of %d bytes with %d bytes of ecc\n", len,
		       maxlen, ecclen);
		printf("Barcode : %s\n", barcode);
		printf("Encoding: %s\n", encoding);
		break;
	case 'h':		// hex
		dumphex(grid, W, H, 0);
		break;
	case 'b':		// bin
		{
			int y;
			for (y = 0; y < H; y++) {
				int v = 0, x, b = 128;
				for (x = 0; x < W; x++) {
					if (grid[y * W + x])
						v |= b;
					b >>= 1;
					if (!b) {
						putchar(v);
						v = 0;
						b = 128;
					}
				}
				if (b != 128)
					putchar(v);
			}
		}
		break;
	case 't':		// text
		{
			int y;
			for (y = H - 1; y >= 0; y--) {
				int x;
				for (x = 0; x < W; x++)
					printf("%c",
					       grid[W * y + x] ? '*' : ' ');
				printf("\n");
			}
		}
		break;
	case 'e':		// EPS
		printf("%%!PS-Adobe-3.0 EPSF-3.0\n"
		       "%%%%Creator: IEC16022 barcode/stamp generator\n"
		       "%%%%BarcodeData: %s\n" "%%%%BarcodeSize: %dx%d\n"
		       "%%%%BarcodeFormat: ECC200\n"
		       "%%%%DocumentData: Clean7Bit\n" "%%%%LanguageLevel: 1\n"
		       "%%%%Pages: 1\n" "%%%%BoundingBox: 0 0 %d %d\n"
		       "%%%%EndComments\n" "%%%%Page: 1 1\n"
		       "%d %d 1[1 0 0 1 -1 -1]{<\n", barcode, W, H, W + 2,
		       H + 2, W, H);
		dumphex(grid, W, H, 0xFF);
		printf(">}image\n");
		break;
	case 's':		// Stamp
		{
			char temp[74], c;
			time_t now;
			struct tm t = {
				0
			};
			int v;
			if (barcodelen < 74) {
				fprintf(stderr,
					"Does not look like a stamp barcode\n");
				return 1;
			}
			memcpy(temp, barcode, 74);
			c = temp[5];
			temp[56] = 0;
			t.tm_year = atoi(temp + 54) + 100;
			t.tm_mday = 1;
			now = mktime(&t);
			temp[54] = 0;
			now += 86400 * (atoi(temp + 51) - 1);
			t = *gmtime(&now);
			temp[46] = 0;
			v = atoi(temp + 36);
			printf("%%!PS-Adobe-3.0 EPSF-3.0\n"
			       "%%%%Creator: IEC16022 barcode/stamp generator\n"
			       "%%%%BarcodeData: %s\n" "%%%%BarcodeSize: %dx%d\n" "%%%%DocumentData: Clean7Bit\n" "%%%%LanguageLevel: 1\n" "%%%%Pages: 1\n" "%%%%BoundingBox: 0 0 190 80\n" "%%%%EndComments\n" "%%%%Page: 1 1\n" "10 dict begin/f{findfont exch scalefont \
				 setfont}bind def/rm/rmoveto load def/m/moveto load \
				 def/rl/rlineto load def\n" "/l/lineto load def/cp/closepath load def/c{dup stringwidth \
				 pop -2 div 0 rmoveto show}bind def\n" "gsave 72 25.4 div dup scale 0 0 m 67 0 rl 0 28 rl -67 0 rl \
				 cp clip 1 setgray fill 0 setgray 0.5 0 translate 0.3 \
				 setlinewidth\n" "32 32 1[2 0 0 2 0 -11]{<\n", barcode, W, H);
			dumphex(grid, W, H, 0xFF);
			printf(">}image\n"
			       "3.25/Helvetica-Bold f 8 25.3 m(\\243%d.%02d)c\n"
			       "2.6/Helvetica f 8 22.3 m(%.4s %.4s)c\n"
			       "1.5/Helvetica f 8 3.3 m(POST BY)c\n"
			       "3.3/Helvetica f 8 0.25 m(%02d.%02d.%02d)c\n",
			       v / 100, v % 100, temp + 6, temp + 10, t.tm_mday,
			       t.tm_mon + 1, t.tm_year % 100);
			if (c == '1' || c == '2' || c == 'A' || c == 'S') {
				if (c == '2')
					printf
					    ("42 0 m 10 0 rl 0 28 rl -10 0 rl cp 57 0 m 5 0 rl 0 \
					   28 rl -5 0 rl cp");
				else
					printf
					    ("42 0 m 5 0 rl 0 28 rl -5 0 rl cp 52 0 m 10 0 rl 0 \
					   28 rl -10 0 rl cp");
				printf
				    (" 21 0 m 16 0 rl 0 28 rl -16 0 rl cp fill\n"
				     "21.3 0.3 m 15.4 0 rl 0 13 rl -15.4 0 rl cp 1 setgray \
					fill gsave 21.3 0.3 15.4 27.4 rectclip newpath\n");
				switch (c) {
				case '1':
					printf
					    ("27/Helvetica-Bold f 27 8.7 m(1)show grestore 0 setgray \
				   1.5/Helvetica-Bold f 22 3.3 m(POSTAGE PAID GB)show \
				   1.7/Helvetica f 29 1.5 m(DumbStamp.co.uk)c\n");
					break;
				case '2':
					printf
					    ("21/Helvetica-Bold f 23.5 13 m(2)1.25 1 scale show grestore \
				   0 setgray 1.5/Helvetica-Bold f 22 3.3 \
				   m(POSTAGE PAID GB)show 1.7/Helvetica f 29 1.5 \
				   m(DumbStamp.co.uk)c\n");
					break;
				case 'A':
					printf
					    ("16/Helvetica-Bold f 29 14.75 m 1.1 1 scale(A)c grestore 0 \
				   setgray 1.5/Helvetica-Bold f 22 3.3 m(POSTAGE PAID GB)show \
				   1.7/Helvetica f 22 1.5 m(Par Avion)show\n");
					break;
				case 'S':
					printf
					    ("10/Helvetica-Bold f 29 17 m(SU)c grestore 0 setgray \
					   1.5/Helvetica-Bold f 22 1.5 m(POSTAGE PAID GB)show\n");
					break;
				}
				printf
				    ("2.3/Helvetica-Bold f 29 10 m(LOYAL MAIL)c\n");
			} else if (c == 'P') {	// Standard Parcels
				printf("21 0 m 41 0 rl 0 28 rl -41 0 rl cp fill\n" "37.7 0.3 m 24 0 rl 0 27.4 rl -24 0 rl cp 1 setgray fill \
					gsave 21.3 0.3 16.4 27.4 rectclip newpath\n" "22.5/Helvetica-Bold f 37.75 -1.25 m 90 rotate(SP)show \
					grestore 0 setgray\n"
				       "3.5/Helvetica-Bold f 49.7 21.5 m(LOYAL MAIL)c\n" "2.3/Helvetica-Bold f 49.7 7 m(POSTAGE PAID GB)c\n" "2.6/Helveica f 49.7 4.25 m(DumbStamp.co.uk)c\n");
			} else if (c == '3')
				printf("21.15 0.15 40.7 27.7 rectstroke\n"
				       "21 0 m 41 0 rl 0 5 rl -41 0 rl cp fill\n"
				       "0 1 2{0 1 18{dup 1.525 mul 22.9 add 24 3 index 1.525 mul \
					add 3 -1 roll 9 add 29 div 0 360 arc fill}for pop}for\n" "50.5 23.07 m 11.5 0 rl 0 5 rl -11.5 0 rl cp fill\n"
				       "5.85/Helvetica f 23.7 15.6 m(Loyal Mail)show\n" "4.75/Helvetica-Bold f 24 11 m(special)show 4.9/Helvetica \
					f(delivery)show\n" "gsave 1 setgray 3.2/Helvetica-Bold f 24 1.6 \
					m(next day)show 26 10.15 m 2 0 rl stroke grestore\n" "21.15 9.9 m 53.8 9.9 l stroke 53.8 9.9 0.4 0 360 \
					arc fill\n");
			printf("end grestore\n");
		}
		break;
	case 'p':		// png
		{
			int x, y;
			Image *i = ImageNew(W + 2, H + 2, 2);
			i->Colour[0] = 0xFFFFFF;
			i->Colour[1] = 0;
			for (y = 0; y < H; y++)
				for (x = 0; x < W; x++)
					if (grid[y * W + x])
						ImagePixel(i, x + 1, H - y) = 1;
			ImageWritePNG(i, fileno(stdout), 0, -1, barcode);
			ImageFree(i);
		}
		break;
	default:
		fprintf(stderr, "Unknown output format %s\n", format);
		break;
	}
	return 0;
}
