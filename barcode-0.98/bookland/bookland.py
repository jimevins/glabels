#!/usr/local/bin/python

MYNAME="bookland.py"
MYVERSION="0.92"
COPYRIGHT="(C) 1999-2001 J. Milgram"
DATE = "Jan. 2002"
MAINTAINER = "bookland-bugs@cgpp.com"

#   Copyright (C) 1999,2000 Judah Milgram     
#
#   bookland.py - generate Bookland EAN symbol for ISBN encoding
#
#   This program is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public License
#   as published by the Free Software Foundation; either version 2
#   of the License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#     
#   You should have received a copy of the GNU General Public License along
#   with this program; if not, write to the Free Software Foundation, Inc.,
#   59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
#
# ==============================================================================
#
#  usage: bookland.py [ISBN] [price-code] > output.eps
#
#         ISBN - the ISBN, with or without check digit, with or without hyphens.
#                default: 1-56592-197-6 ("Programming Python"). If the check digit
#                is provided on the command line, it is verified. If not, it is
#                calculated. It's up to you to get the hyphenation right - it's
#                important, and something the program can't calculate for you.
#
#         price - the five digit add-on code. Usually used to indicate the price,
#                 in which case the first digit indicates the currency (4=$CAN,
#                 5=$US, etc.). The remaining digits indicate the price, with
#                 decimal point assumed to be between the digit 3 and 4.
#                 For example: $US 6.95 = 50695, $CAN 35.00 = 43500. Instead of a
#                 price code, a 5 digit add-on ranging from 90000-98999 can be
#                 used for internal purposes. BISG recommends just using 90000 if
#                 you don't want to specify a price. Add-ons ranging from 99000 to
#                 99999 have been reserved for special use.
#
#  An Encapsulated Postscript file (eps) is sent to standard out. This may in turn
#  be converted to other formats using the pbmplus package. You may have trouble
#  getting the OCRB to map correctly. If you already have the font, you can look in
#  the Fontmap file to see what your system calls it, and edit the fontnames accordingly
#  (see below). If you don't have it, you might find it on your DOS system. You
#  need a .pfa/.pfb (Type 1) or .ttf (TrueType). Your Postscript interpreter might
#  or might not be able to deal with TrueType. In any event, in an emergency, you
#  might get away with Helvetica. Note that as of 1990 BISG no longer requires the
#  ISBN to be printed in OCR-A.
#
#  Take the "no-warranty" disclaimer seriously. Going to print with a faulty bar
#  can cost you a bundle, and you'll be on your own. It's up to you to verify that
#  the symbol is valid. If you need "corporate accountability", try the Book
#  Industry Study Group at (212) 929-1393 or the US ISBN Agency at (908) 665-6770
#  and ask for a list of commercial vendors. Outside the US, don't know.
#
#  Feedback welcome. If you discover a case where the program generates a faulty
#  symbol, I definitely want to hear about it - write me at milgram@cgpp.com or
#  P.O. Box 8376, Langley Park, MD 20787, USA
#
#  INSTALLATION:
#
#  If you have a Python interpreter on your system, you're done. Just put this file
#  somewhere in your path and give it execute permission. If you haven't installed
#  Python, see http://www.python.org. It has been ported to Macs, DOS, and MS-Windows.
#
#  ABOUT THE BOOKLAND EAN
#
#  The most difficult part of this project was finding the documents that define
#  the Bookland EAN. There appears to be no single, authoritative source that
#  provides all the information required. Some good sources:
#
#  [1] "Machine-Readable Coding Guidelines for the U.S. Book Industry", Book
#      Industry Study Group, New York, Jan., 1992. (212) 929-1393
#  [2] "UPC Symbol Specification Manual", Uniform Code Council Inc.,
#      Dayton, Ohio, January 1986 (May 1995 Reprint). (937) 435-3870; I found it
#      at http://www.uc-council.org/d36-t.htm
#  [3] "EAN Identification for Retail/Trade Items", EAN International. I found it
#      in Feb. 1999 at http://www.ean.be/html/Numbering.html
#  [4] "Hyphenation Instructions", web page at:
#      http://www.isbn.org/standards/home/isbn/international/hyphenation-instructions.asp
#
#  The starting point of the exercise is the ISBN, assigned by the national ISBN
#  Agency. This is a 10 digit number, the last being a check digit. The ISBN is
#  converted to a 13 digit EAN number. The first three digits of the EAN-13 indicate
#  the country or region. A prefix of 978 has been assigned to books, regardless
#  of country of origin (hence, "Bookland") [3]. The remaining ten digits are the
#  first 9 digits of the ISBN followed by the EAN-13 check digit.
#
#  It seems the EAN-13 check digit can be calculated using the same algorithm as the
#  UPC Version A number. Note that the EAN-13 check digit is always between 0 and 9,
#  compare with ISBN check digit which can range to 10 ("X"). See Reference [2],
#  Section 2 and Appendix G for details of creation of the EAN-13 symbol. Table 2 of
#  Appendix G provides a good comparison of the UPC-A and EAN-13 symbols.
#
#  The 5 digit add-on (here called, "UPC5") is defined in Ref. [2] Appendix D.
#  The ">" to the right of the five digit code serves to enforce the "quiet zone" to
#  the right of the bar pattern. Can't remember where I read that. It's probably
#  optional. According to [1], in the UK, three horizontal bars appear over price
#  add-ons. Haven't implemented that here. The UPC5 encoding is based on UPC-A and
#  UPC-E.
#
#  According to [2], Section 3, the EAN-13 numbers and 5-digit add-ons are supposed
#  to be printed in OCR-B. The ISBN itself is printed above the EAN-13 symbol. At
#  one time it was to be printed in OCR-A, but as of 1990 this requirement has been
#  dropped [1], and I assume this means you can use any font you like.
#
#  SEE ALSO:
#
#  "TinyHelp 5 - Making ISBN Barcodes", D. Byram-Wigfield. Another approach to making
#                the ISBN barcode symbol. I saw it at
#                http://www.cappella.demon.co.uk/index.html/
#                but haven't tried it.
#
#  "XBarcode" - nice open-source X-Windows program for generating all sorts of bar codes.
#               It does much more than this program, but didn't seem to do the UPC
#               5-digit add-on or do the ISBN->EAN13 calculation (as of v. 2.11). Might
#               have made more sense to add this capability, but I needed a Python project.
#               In any event, their license forbids distribution in modified form!
#
#  HYPHENATION
#
#  bookland.py includes automatic hyphenation for ISBN's in group 0 and 1
#  (English-language). This is based on my reading of Ref [4]. If in doubt,
#  users can use the "-y" option to force the program to accept the hyphenation
#  as input. For other ISBN groups and for ISMN's, no hyphenation is performed
#  other than to ensure a hyphen is placed after the group identifier and before
#  the check digit.
#
#  ABOUT THE ISMN:
#
#  See the ISMN Users' Manual, 3rd Edition, 1998, ISBN 3-88053-070-X, published by
#  the International ISMN Agency, Staatsbibliothek Preussischer Kulturbesitz, Berlin.
#  I found my copy at http://www.ismn.spk-berlin.de/download/ISMNmanual.pdf
#
#  An ISMN is just like an ISBN, except:
#   - first character is an "M"
#   - the "M" counts as a "3" for computing the ISMN check digit (last digit)
#   - the checksum weights are 3,1,3,1,3,1,3,1,3, sum to be divisible by "10". This
#     means the last character is always a numerical digit, never an "X".
#   - the EAN number is "979" plus the *entire* ten character ISMN, except the
#     "M" is replaced by "0". Note this means the ISMN checksum is identical to the
#     corresponding EAN-13 checksum (excercise left to the reader).
#
#  When bookland.py detects an "M" in the first position of the ISBN, it interprets
#  it as an ISMN and proceeds accordingly. The 5-digit price code symbol is suppressed.
#
#  BAR WIDTH REDUCTIONS
#
#  Starting Version 0.92, the widths of the individual bars can be reduced using the
#  "-r" option (units are inches). This is to compensate for bleed during printing.
#  I don't know when it's a good idea to actually use this; in any event consult with
#  your printer first. If not input, it defaults to zero (no reduction).
#
#  ABOUT PYTHON
#
#  See http://www.python.org
#
#  TO DO:
#
#  - Generalize to more bar codes, starting with UPC-A and UPC-E. "Plain" EAN13 is
#    already built in, could add command line argument to generate that instead of
#    Bookland.
#  - Make font sizes and placement easier to configure - not sure I have it right.
#    Does human-readable 5-digit code take wider font spacing?
#  - Clean up bounding box stuff.
#  - Bells and whistles.
#  - GUI?
#
#  HISTORY:
#
#  1/2002  -  v 0.92 add ISMN support (thanks to Lars-Henrik Nysten for this suggestion)
#                    re-wrote bar generation to preclude possibility of white hairlines
#                        between adjacent black modules. Thanks to Tero Lindfors for
#                        reporting this bug.
#                    new -o option to write eps to file rather than stdout
#                    new -x option for "check only" (verifies check digit)
#                    new -r option for bar width reduction (compensate for print bleed)
#                    new -s option to scale module (bar) height (Lars-Henrik Nysten again)
#                    can suppress UPC-5 price code by entering empty string. (thanks to
#                        Jacques Du Pasquier for this suggestion)
#                    re-wrote ISBN/ISMN sanity checks
#                    lowercase alphas ("x" for ISBN and "m" for ISMN) now ok on input
#                    fix "long" command line options.
#  10/2001 -  v 0.91 add -z option for quiet zone ">"
#                    add -f option for fonts
#                    re-write command line parsing to use getopt
#  1/2000  -  v 0.09 eliminate use of eval
#  9/99    -  v 0.08 accomodate different versions of OCRB by fitting
#                    all strings to prescribed width. Thanks to Toby Gadd
#                    for reporting this problem and Peter Deutsch for
#                    help finding the fix.
#  7/99    -v0.05-0.07 improve error handling.
#  3/27/99 - v0.04 add "--help" and "--version".
#  3/13/99 - v0.03, do a showpage at end (it's allowed)
#            fixed checksum calculations for certain cases
#  2/7/99 - v0.02, fixed LH parity pattern for EAN13. It's not the check digit!
#  2/7/99 - initial release
# ================================================================================

#
#  barCodeSymbol - the whole printed symbol, including bar code(s) and product code(s).
#  UPC, UPCA, UPC5, EAN13 - the number itself, with check digit, string representation,
#                         and barcode bits
#

import re       # we should get rid of regsub and regex in favor of re
                # hope there's no conflict.
import string
import sys
import regsub
from regex_syntax import *
import regex
regex.set_syntax(RE_SYNTAX_AWK)
from types import *

BooklandError = "Something wrong"


A="A";B="B";C="C";O="O";E="E"
UPCABITS = [{O:"0001101",E:"1110010"},
            {O:"0011001",E:"1100110"},
            {O:"0010011",E:"1101100"},
            {O:"0111101",E:"1000010"},
            {O:"0100011",E:"1011100"},
            {O:"0110001",E:"1001110"},
            {O:"0101111",E:"1010000"},
            {O:"0111011",E:"1000100"},
            {O:"0110111",E:"1001000"},
            {O:"0001011",E:"1110100"}]
UPCAPARITY = [ "OOOOOOEEEEEE" ] * 10
UPCEBITS = [{O:"0001101",E:"0100111"},
            {O:"0011001",E:"0110011"},
            {O:"0010011",E:"0011011"},
            {O:"0111101",E:"0100001"},
            {O:"0100011",E:"0011101"},
            {O:"0110001",E:"0111001"},
            {O:"0101111",E:"0000101"},
            {O:"0111011",E:"0010001"},
            {O:"0110111",E:"0001001"},
            {O:"0001011",E:"0010111"}]
# what about UPCEPARITY? Don't need for isbn.
UPC5BITS = UPCEBITS
UPC5PARITY = ["EEOOO","EOEOO","EOOEO","EOOOE","OEEOO",
              "OOEEO","OOOEE","OEOEO","OEOOE","OOEOE"]
EAN13BITS = [{A:"0001101", B:"0100111", C:"1110010"},
             {A:"0011001", B:"0110011", C:"1100110"},
             {A:"0010011", B:"0011011", C:"1101100"},
             {A:"0111101", B:"0100001", C:"1000010"},
             {A:"0100011", B:"0011101", C:"1011100"},
             {A:"0110001", B:"0111001", C:"1001110"},
             {A:"0101111", B:"0000101", C:"1010000"},
             {A:"0111011", B:"0010001", C:"1000100"},
             {A:"0110111", B:"0001001", C:"1001000"},
             {A:"0001011", B:"0010111", C:"1110100"}]
EAN13PARITY = map(lambda x: x+"CCCCCC",
                  ["AAAAAA","AABABB","AABBAB","AABBBA","ABAABB",
                   "ABBAAB","ABBBAA","ABABAB","ABABBA","ABBABA"])

PSFORMAT = "%.6f"
# Default fonts.
# Fonts might have a different name on your system.
# Edit if required.
ISBNFONT = "OCRB"      #  Doesn't have to be OCR-B
EAN13FONT = "OCRB"
UPC5FONT = "OCRB"

class psfile:
    
    def __init__(self):
        self.x0 = 100; self.y0 = 100
        self.lines=[]
        self.bb=[self.x0,self.y0,self.x0,self.y0]

    def orbb(self,arg):
        self.bb[0] = min(self.bb[0],self.x0+arg[0])
        self.bb[1] = min(self.bb[1],self.y0+arg[1])
        self.bb[2] = max(self.bb[2],self.x0+arg[2])
        self.bb[3] = max(self.bb[3],self.y0+arg[3])

    def translate(self,dx,dy):
        self.x0 = self.x0 + dx
        self.y0 = self.y0 + dy
        return "%d %d translate 0 0 moveto" % (dx,dy)
        
    def out(self,file=None):
        if file:
            outfid=open(file,"w")
        else:
            outfid=sys.stdout
        for line in self.lines:
            outfid.write("%s\n"%line)
        outfid.close()

    def do(self,arg):
        self.lines = self.lines + arg

    def setbb(self):
        for i in range(len(self.lines)):
            if self.lines[i]=="%%BoundingBox: TBD":
                self.lines[i]= "%%BoundingBox:" + \
                               " %d"%self.bb[0] + \
                               " %d"%self.bb[1] + \
                               " %d"%self.bb[2] + \
                               " %d"%self.bb[3]
                return

    def header(self,title,comments,ean13font,isbnfont,upc5font):
        for i in range(len(comments)):
            comments[i] = regsub.gsub("^","%  ",comments[i])
        # There's a more elegant way to do the bounding box line:
        return [ "%!PS-Adobe-2.0 EPSF-1.2",
                 "%%Creator: " + MYNAME + "  " + MYVERSION + "  " + DATE,
                 "%%Title: " + title,
                 "%%BoundingBox: TBD",
                 "%%EndComments" ] +\
                 comments + \
               [ "\n% These font names might be different on your system:",
                 "/ean13font { /" + ean13font + " findfont 10 scalefont setfont } def",
                 "/isbnfont { /" + isbnfont + " findfont 8 scalefont setfont } def",
                 "/upc5font { /" + upc5font +" findfont 14 scalefont setfont } def\n",
                 "/nextModule { moduleWidth 0 rmoveto } def",
                 "% The following shenanigans is to deal with different implementations",
                 "% of same font having different char sizes and spacing.",
                 "% function fitstring:",
                 "% usage: width string font fitstring",
                 "% set font, scaled so that string exactly fits desired width",
                 "% leave string on stack",
                 "/fitstring { dup findfont 1 scalefont setfont % w s f",
                 "3 1 roll % f w s",
                 "dup stringwidth pop % f w s sw",
                 "3 2 roll exch div % f s x",
                 "3 2 roll findfont exch scalefont setfont",
                 "} def",
                 "/barHeight { 72 } def",
                 "/nextModule { moduleWidth 0 rmoveto } def",
                 "/topcentershow {dup stringwidth pop neg 2 div -9 rmoveto show} def",
                 "/toprightshow {dup stringwidth pop neg -9 rmoveto show} def",
                 "/bottomcentershow {dup stringwidth pop neg 2 div 0 rmoveto show} def",
                 "/bottomrightshow {dup stringwidth pop neg 0 rmoveto show} def",
                 "/W { moduleWidth mul 0 rmoveto } def",
                 "/B { dup moduleWidth mul 2 div 0 rmoveto",
                 "dup moduleWidth mul barWidthReduction sub setlinewidth",
                 "0 barHeight rlineto 0 barHeight neg rmoveto",
                 "currentpoint stroke moveto",
                 "moduleWidth mul 2 div 0 rmoveto } def",
                 "/L { dup moduleWidth mul 2 div 0 rmoveto",
                 "dup moduleWidth mul barWidthReduction sub setlinewidth",
                 "0 -5 rmoveto 0 5 rlineto",
                 "0 barHeight rlineto 0 barHeight neg rmoveto",
                 "currentpoint stroke moveto",
                 "moduleWidth mul 2 div 0 rmoveto } def",
                 self.x0,self.y0,"translate",
                 "0 0 moveto" ]

    def trailer(self):
        return ["stroke","% showpage supposedly OK in EPS",
                "showpage","\n% Good luck!"]

        
class UPC:
    
    # Includes UPC-A, UPC-E, EAN-13 (sorry), UPC-5 et al.

    def __init__(self,arg):
        # arg is a string, either:
        # - product code including checksum
        # - same, with hyphens (hyphens not verified)
        # - same, but with last digit (checksum) dropped, possibly leaving a
        #   trailing hyphen.
        # If checksum is included, it will be verified.        
        # N.B. "integer" representation is still a string! Just has no hyphens.

        self.s=arg
        self.verifyChars(self.s)
        self.n = regsub.gsub("-","",self.s)    # create "integer" representation
        self.x = self.checkDigit(self.n)       # always calculate check digit
        if len(self.n) == self.ndigits:
            self.verifyCheckDigit()                # if check digit given, verify it
        elif len(self.n) == self.ndigits-1:
            self.tackonCheckDigit()                # tack on check digit
        else:
            raise BooklandError, "UPC: wrong number of digits in \"" + self.s + "\""

    def setbits(self,arg):                       # UPC (all)
        self.bits=""
        parityPattern=self.parityPattern()
        bitchar=self.bitchar()
        for p in range(len(arg)):
            digit=int(arg[p])
            # maybe better to define parityPattern with a leading blank?
            parity=parityPattern[p]
            bit=bitchar[digit][parity]
            self.bits=self.bits + bit

    def verifyChars(self,s):                     # UPC (all)
        # Trailing hyphen allowed.
        nevergood = "--|^-|[^0-9-]"
        ierr=regex.search(nevergood,s)
        if ierr != -1:
            raise BooklandError, \
                  "UPCA: in %s: illegal characters beginning with: %s" % (s,s[ierr])

    def verifyCheckDigit(self):               # UPC (all)
        # first verify correct number of digits.
        soll=self.checkDigit(self.n)
        ist=self.s[-1:]
        if ist != soll:
            raise BooklandError, "For %s checksum %s is wrong, should be %s" % \
                             (self.s,ist,soll)

    def xstring(self,p):                      # UPC (all)
        return "%d" % p

    def tackonCheckDigit(self):
        self.n = self.n + self.x              # UPC (all)
        self.s = self.s + self.x

class UPCA(UPC):

    def __init__(self,arg):
        UPC.__init__(self,arg)
        self.setbits(self.n[1:])                 # skip first digit
        
    def parityPattern(self):
        return UPCAPARITY[int(self.x)]
    def bitchar(self):
        return UPCABITS
    
    def checkDigit(self,arg):               # UPCA/EAN13
          weight=[1,3]*6; magic=10; sum = 0
          for i in range(12):         # checksum based on first 12 digits.
              sum = sum + int(arg[i]) * weight[i]
          z = ( magic - (sum % magic) ) % magic
          if z < 0 or z >= magic:
              raise BooklandError, "UPC checkDigit: something wrong."
          return self.xstring(z)


class ISBN:
    # Includes ISMN, if the plan falls together.
    def __init__(self,arg):
        self.ndigits=10            #  Includes check digit!
        self.s=string.upper(arg)
        self.n=re.sub("[ -]","",self.s) # "integer" representation
        # In ISMN, I allow spaces in place of hyphens. See ISMN User's manual.
        if re.match("^M( |-)?\d(( |-)?\d){7,7}(-| )?\d?$",self.s):
            # ISMN
            self.name="ISMN"
            self.n=re.sub("^M","3",self.n)
            self.weight=[3,1,3,1,3,1,3,1,3]
            self.magic=10
        elif re.match("^\d-?\d(-?\d){7,7}-?(\d|X)?$",self.s):
            # ISBN
            self.name="ISBN"
            self.weight=[10,9,8,7,6,5,4,3,2]
            self.magic=11
        else:
            raise BooklandError, "%s invalid (hyphenation, characters, or length)" % self.s
        self.x = self.checkDigit()
        if len(self.n) == self.ndigits:
            self.verifyCheckDigit()                # if check digit given, verify it
        elif len(self.n) == self.ndigits-1:
            self.tackonCheckDigit()                # tack on check digit
        else:
            raise BooklandError, "%s failed. Please report as bug" % self.s


    def checkDigit(self):     # ISBN and ISMN; UPCA/EAN13 similar but for weights etc.
        # now that we're checking regex's in init, we don't have to check the
        # argument at all. (used to check length and bad characters)
        sum = 0
        for i in range(9):      # checksum based on first nine digits.
            sum = sum + int(self.n[i]) * self.weight[i]
        z = ( self.magic - (sum % self.magic) ) % self.magic
        if z < 0 or z >= self.magic:
            raise BooklandError, \
                  "%s: checksum %d is wrong - please report as bug" % (self.s,z)
        return self.xstring(z)

    def xstring(self,p):
        if p == 10:
            return "X"
        else:
            return "%d" % p

    def tackonCheckDigit(self):
        if self.s[-1:] == "-":
            # Already have a trailing hyphen
            self.s = self.s + self.x
        else:
            self.s = self.s + "-" + self.x

    def verifyCheckDigit(self):               # UPC A; EAN13
        # first verify correct number of digits.
        soll=self.x
        ist=self.s[-1:]
        if ist != soll: raise BooklandError, \
                  "For %s checksum %s is wrong, should be %s\n" % (self.s,ist,soll)

class Bar:
    # a run of adjacent modules of identical value.
    def __init__(self,val):
        self.val=val
        if not self.val in "L01":
            raise BooklandError, "bar bit: %s, pls report as a bug" % self.val
        self.width=1
        if self.val=="1":
            self.color="Black"
        elif self.val=="0":
            self.color="White"
        elif self.val=="L":
            self.color="Long Black"
    def __cmp__(self,other):
        if self.val==other or (self.val=="L" and other=="1"):
            return 0
        else:
            return 1
    def inc(self):
        self.width=self.width+1
    def pslines(self):
        if self.val=="L":
            rval = [ "%d L " % self.width ]
        elif self.val=="1":
            rval = [ "%d B " % self.width ]
        else:
            rval = [ "%d W " % self.width ]
        return rval
    def __repr__(self):
        return "%s bar of width %d" % (self.color,self.width)

class barCodeSymbol:

    def __init__(self):
        self.patternWidth = len(self.bits)*self.moduleWidth
        # Anything else?

    def bitsComment(self):
        return [ "%% Bits:\n%% %s" % self.bits ]
        
    def psbars(self):
        # new version, try to prevent all hairlines between adjacent modules.
        bars = []
        bar=Bar(self.bits[0])
        for bit in self.bits[1:]:
            if bit==bar:
                bar.inc()
            else:
                bars.append(bar)
                bar=Bar(bit)
        bars.append(bar)
        rval = ["0 0 moveto"]
        for bar in bars:
            rval = rval + bar.pslines()
        rval = rval + [ "stroke" ]
        return rval

    def psbarsold(self):
        psbits=regsub.gsub("1","I ",self.bits)
        psbits=regsub.gsub("0","O ",psbits)
        psbits=regsub.gsub("L","L ",psbits)
        linewidth=50
        p=0; j=linewidth; m=len(psbits); psbarlines=[]; blanks="^ | $"
        while p <= m:
            j = min(linewidth,m-p)
            psbarlines = psbarlines + [ regsub.gsub(blanks,"",psbits[p:p+j]) ]
            p=p+linewidth
        return [ "0 0 moveto" ] + psbarlines + [ "stroke" ]
        
    def psSetBarHeight(self):
        return [ "/barHeight { " + PSFORMAT % self.moduleHeight + " 72 mul } def" ]

    def psSetBarWidthReduction(self):
        return [ "/barWidthReduction { " + \
                 PSFORMAT % self.barWidthReduction + " 72 mul } def" ]

    def psSetModuleWidth(self):
        rval = [ "/moduleWidth { " + PSFORMAT % self.moduleWidth + " 72 mul } def" ]
        return rval

    def psBottomRightText(self,text,font):
        # this is specifically for the upc5 price code.
        # this is all starting to get messy.
        return [ PSFORMAT % self.patternWidth + " 72 mul dup 2 div",
                 PSFORMAT % self.moduleHeight + " 72 mul 2 add moveto",
                 "(" + text + ") /" + font + " fitstring bottomcentershow" ]    

    def psTopCenterText(self,text,font):
        # the text at the top center of the bar pattern (i.e. the ISBN)
        return [ PSFORMAT % self.patternWidth + " 72 mul dup 2 div",
                 PSFORMAT % self.moduleHeight + " 72 mul 3 add moveto",
                 " (" + text + ") /" + font  + " fitstring bottomcentershow" ]

    def psFittedText(self,width,text,font):
        return [ PSFORMAT % width + " (" + text + ") " + font + " fitstring" ]

    # This is optional; serves to enforce quiet zone to right of UPC 5 add-on
    def psGreaterThan(self,font):
        return [ PSFORMAT % self.patternWidth + " 72 mul",
                 PSFORMAT % self.moduleHeight + " 72 mul 2 add moveto",
                 "/%s (>) show" % font ]

class EAN13Symbol(barCodeSymbol):

    def __init__(self,arg,font=EAN13FONT,heightMultiplier=1,barWidthReduction=0):
        # arg is a string with the EAN product code
        self.barWidthReduction=barWidthReduction
        self.ean13 = EAN13(arg)
        self.moduleWidth = 0.0130
        specModuleHeight = 1.00
        self.moduleHeight = 1.00 * heightMultiplier
        self.bits = self.ean13.bits
        barCodeSymbol.__init__(self)
        self.font=font

    def bb(self):
        return  [ -12, -10, self.patternWidth*72+10, self.moduleHeight*72+12 ]

    def pslines(self):
        return self.bitsComment() + \
               self.psSetModuleWidth() + \
               self.psSetBarWidthReduction() + \
               self.psSetBarHeight() + \
               self.psbars() + \
               self.psLRDigitLines()
              
    def psLRDigitLines(self):
        # 24 = 3+6*7/2
        # 70 = 3+6*7+4+6*7/2   "4" so we center on the "L" bars (the rightmost of
        #      the center guard bars is an "O".
        # "5" in check digit is the five-module spacing recommended by [2], section 3.
        return [ "% We do the left digits first and leave the font scaled",
                 "% as is for the 9-digit and the right-digits.",
                 "% EAN13 Left Digits:",
                 "moduleWidth 24 mul 0 moveto",
                 "moduleWidth 40 mul (" + self.ean13.leftDigits + ") ",
                 "/" + self.font + " fitstring topcentershow",
                 "\n% EAN13 human-readable number",
                 "% The \"9\" digit (only when encoding ISBN's and ISMN's, I think):",
                 "-5 0 moveto (" + self.ean13.n[0] + ") toprightshow",                 
                 "% EAN13 Right Digits:",
                 "moduleWidth 70 mul 0 moveto",
                 "moduleWidth 40 mul (" + self.ean13.rightDigits + ") topcentershow" ]

class EAN13(UPCA):

    def __init__(self,arg):
        self.ndigits=13            #  Includes check digit!
        UPCA.__init__(self,arg)
        leftBits = self.bits[0:42]
        rightBits = self.bits[42:]
        leftGuard="L0L"
        rightGuard="L0L"
        center="0L0L0"
        self.bits = leftGuard + leftBits + center + rightBits + rightGuard
        self.leftDigits = self.n[1:7]
        self.rightDigits = self.n[7:13]

    def parityPattern(self):
        # N.B. parity pattern based on leftmost digit, the UCC Spec calls this
        # the "13th" digit. It's not the check digit!
        return EAN13PARITY[int(self.n[0])]
    def bitchar(self):
        return EAN13BITS

class UPC5Symbol(barCodeSymbol):

    def __init__(self,arg,heightMultiplier=1,barWidthReduction=0):
        # arg is a string with the 5 digit add-on.
        self.barWidthReduction=barWidthReduction
        self.upc5 = UPC5(arg)
        self.moduleWidth = 0.0130
        specModuleHeight = 0.852
        self.moduleHeight = 0.852 * heightMultiplier
        self.bits = self.upc5.bits
        barCodeSymbol.__init__(self)

    def pslines(self):
        return self.bitsComment() + \
               self.psSetModuleWidth() + \
               self.psSetBarHeight() + \
               self.psbars()

    def bb(self):
        # Note quiet zone is there even if we don't print the ">".
        return  [ 0, 0, self.patternWidth*72+10, self.moduleHeight*72+10 ]

UPC5Error = "Something wrong with 5-digit price code add-on."
class UPC5(UPC):

    def __init__(self,arg):
        self.ndigits=5            #  Includes check digit!
        p=re.search("[^0-9]",arg)
        if p:
          badchar=arg[p.start()]
          raise UPC5Error, "\"%s\" is wrong. The character \"%s\" is not allowed. Price code add-on should contain %d digits and nothing else. Or leave blank to suppress the UPC-5 code." % (arg,badchar,self.ndigits)
        elif len(arg) != self.ndigits:
          raise UPC5Error, \
            "\"%s\" is wrong. Price code add-on should have exactly %d digits." % (arg,self.ndigits)
        UPC.__init__(self,arg)
        self.setbits(self.n)
        leftGuard="1011"
        # no right guard for UPC 5-digit add-on
        # Have to insert pesky delineators:
        delineator = "01"
        self.bits = leftGuard + \
                    self.bits[0:7] + delineator + \
                    self.bits[7:14] + delineator + \
                    self.bits[14:21] + delineator + \
                    self.bits[21:28] + delineator + \
                    self.bits[28:35]

    def checkDigit(self,arg):     # UPC5
          weight=[3,9,3,9,3]; sum = 0
          for i in range(5):
              sum = sum + int(arg[i]) * weight[i]
          return self.xstring(sum % 10)

    def verifyCheckDigit(self):               # UPC2/5 checksum not in number
        return

    def parityPattern(self):
        return UPC5PARITY[int(self.x)]
    def bitchar(self):
        return UPC5BITS

class bookland(barCodeSymbol):

    def __init__(self,isbn,price="",*rest):


        # Some defaults:
        ean13font=EAN13FONT
        isbnfont=ISBNFONT
        upc5font = UPC5FONT
        zone=None
        heightMultiplier=1.0
        commandLine=""
        barWidthReduction=0

        # Maybe different fonts:
        if len(rest)>0:
            font=rest[0]
            if font:
                ean13font=font
                isbnfont=font
                upc5font=font
        if len(rest)>1:
            zone=rest[1]
        if len(rest)>2:
            heightMultiplier=float(rest[2])
        if len(rest) > 3:
            commandLine=rest[3]
        if len(rest) > 4:
            barWidthReduction=float(rest[4])

        # Initial setup:
        
        self.ps = psfile()
        self.isbn = ISBN(isbn)

        # Header, EAN13 bars, EAN13 number, and ISBN:

        if self.isbn.name=="ISMN":
            self.ean13Symbol = EAN13Symbol("9790"+self.isbn.n[1:9],ean13font,heightMultiplier,barWidthReduction)
        elif self.isbn.name=="ISBN":
            self.ean13Symbol = EAN13Symbol("978"+self.isbn.n[:9],ean13font,heightMultiplier,barWidthReduction)
        else:
            raise BooklandError, "Internal error doing %s, please report as bug" % isbn

        self.ps.orbb(self.ean13Symbol.bb())
        comments = ["",
                    "     This is free software and comes with NO WARRANTY WHATSOVER",
                    "     Think twice before going to press with this bar code!",
                    "",
                    "Command line: %s" % commandLine,
                    "" ]
        self.ps.lines = self.ps.header(self.isbn.s,comments,ean13font,isbnfont,upc5font) + \
                        [ "ean13font" ] + \
                        self.ean13Symbol.pslines() +\
                        [ "isbnfont" ] + \
                        self.ean13Symbol.psTopCenterText("%s %s" % (self.isbn.name,self.isbn.s),isbnfont)

        # 5-digit add-on:  (optional for ISBN only)
        BLANK=re.compile("^ *$")
        if self.isbn.name=="ISBN" and not BLANK.match(price):
            # 105 = 95 + 10; 10 = separation (min is 9)        
            translate=[ self.ps.translate( self.ean13Symbol.moduleWidth * 72 * 105, 0 ) ]
            self.upc5Symbol = UPC5Symbol(price,heightMultiplier,barWidthReduction)
            self.ps.orbb(self.upc5Symbol.bb())
            self.ps.lines = self.ps.lines + \
                            translate + \
                            self.upc5Symbol.pslines() + \
                            [ "upc5font" ] +\
                            self.upc5Symbol.psBottomRightText(price,upc5font)
            if zone: self.ps.lines=self.ps.lines + self.upc5Symbol.psGreaterThan(upc5font)
        else:
            self.ps.lines.append("%% Skipping UPC-5 price code symbol per request")

        self.ps.lines=self.ps.lines + self.ps.trailer()

        # Can now set bounding box.

        self.ps.setbb()
    
# Here we go ...
        
if __name__ == '__main__':

    def printUsage():
        print "Usage: bookland [-h|--help] [-v|--version] [-x|--check] [-f|--font=<font>] [-s|--height=<height scale>] [-r|--reduction=<reduction factor>] [-o|outfile=<filename>] [-z|--quietzone] [<isbn>|<isbn> <price>]"
        print "Report bugs to " + MAINTAINER

    def printVersion():
        sys.stderr.write("%s version %s %s.\n" % (MYNAME,MYVERSION,COPYRIGHT))
        sys.stderr.write("Bugs to %s\n" % MAINTAINER)
        sys.stderr.write("This is free software and comes with NO WARRANTY\n")

    import getopt
    try:
        opts,args = getopt.getopt(sys.argv[1:],
                                  "xr:s:uvf:hzo:",
                                  ["reduction=","outfile=","height=","noupc",
                                   "check","version","help","font=","quietzone"])
    except:
        printUsage()
        sys.exit(0)

    # some initial defaults:
    isbn = "1-56592-197-6" # Mark Lutz, "Programming Python",
                           # O'Reilly, Sebastopol CA, 1996
    price = "90000"
    font=None
    zone=None
    checkonly=None
    outfile=None
    heightMultiplier=1
    commandLine = string.join(sys.argv)
    barWidthReduction = 0

    # parse command line:
    for opt,val in opts:
        if opt in ("-v","--version"):
            printVersion()
            sys.exit(0)
        elif opt in ("-h","--help"):
            printUsage()
            sys.exit(0)
        elif opt in ("-f","--font"):
            font=val
        elif opt in ("-z","--quietzone"):
            zone=1
        elif opt in ("-x","--check"):
            checkonly=1
        elif opt in ("-s","--height"):
            heightMultiplier = float(val)
        elif opt in ("-r","--reduction"):
            barWidthReduction = val
        elif opt in ("-o","--outfile"):
            outfile=val
    if len(args)==1:
        isbn=args[0]
    elif len(args)==2:
        isbn=args[0]
        price=args[1]

    # Do stuff.

    printVersion()
    try:
        b = bookland(isbn,price,font,zone,heightMultiplier,
                     commandLine,barWidthReduction)
        if not checkonly: b.ps.out(outfile)
        if outfile:
            sys.stderr.write("Output written to %s\n" % outfile)
    except BooklandError, message:
        sys.stderr.write(BooklandError + ": " + message + "\n")
        sys.exit(1)
