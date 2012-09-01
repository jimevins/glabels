/*  barcode_code39_extended.vala
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


using GLib;

namespace glbarcode
{

	public class BarcodeCode39Extended : BarcodeCode39
	{

		private const string[] ascii_map =
		{
			/* NUL */ "%U",   /* SOH */ "$A",   /* STX */ "$B",   /* ETX */ "$C",
			/* EOT */ "$D",   /* ENQ */ "$E",   /* ACK */ "$F",   /* BEL */ "$G",
			/* BS  */ "$H",   /* HT  */ "$I",   /* LF  */ "$J",   /* VT  */ "$K",
			/* FF  */ "$L",   /* CR  */ "$M",   /* SO  */ "$N",   /* SI  */ "$O",
			/* DLE */ "$P",   /* DC1 */ "$Q",   /* DC2 */ "$R",   /* DC3 */ "$S",
			/* DC4 */ "$T",   /* NAK */ "$U",   /* SYN */ "$V",   /* ETB */ "$W",
			/* CAN */ "$X",   /* EM  */ "$Y",   /* SUB */ "$Z",   /* ESC */ "%A",
			/* FS  */ "%B",   /* GS  */ "%C",   /* RS  */ "%D",   /* US  */ "%E",
			/* " " */ " ",    /* !   */ "/A",   /* "   */ "/B",   /* #   */ "/C",
			/* $   */ "/D",   /* %   */ "/E",   /* &   */ "/F",   /* '   */ "/G",
			/* (   */ "/H",   /* )   */ "/I",   /* *   */ "/J",   /* +   */ "/K",
			/* ,   */ "/L",   /* -   */ "-",    /* .   */ ".",    /* /   */ "/O",
			/* 0   */ "0",    /* 1   */ "1",    /* 2   */ "2",    /* 3   */ "3",
			/* 4   */ "4",    /* 5   */ "5",    /* 6   */ "6",    /* 7   */ "7",
			/* 8   */ "8",    /* 9   */ "9",    /* :   */ "/Z",   /* ;   */ "%F",
			/* <   */ "%G",   /* =   */ "%H",   /* >   */ "%I",   /* ?   */ "%J",
			/* @   */ "%V",   /* A   */ "A",    /* B   */ "B",    /* C   */ "C",
			/* D   */ "D",    /* E   */ "E",    /* F   */ "F",    /* G   */ "G",
			/* H   */ "H",    /* I   */ "I",    /* J   */ "J",    /* K   */ "K",
			/* L   */ "L",    /* M   */ "M",    /* N   */ "N",    /* O   */ "O",
			/* P   */ "P",    /* Q   */ "Q",    /* R   */ "R",    /* S   */ "S",
			/* T   */ "T",    /* U   */ "U",    /* V   */ "V",    /* W   */ "W",
			/* X   */ "X",    /* Y   */ "Y",    /* Z   */ "Z",    /* [   */ "%K",
			/* \   */ "%L",   /* ]   */ "%M",   /* ^   */ "%N",   /* _   */ "%O",
			/* `   */ "%W",   /* a   */ "+A",   /* b   */ "+B",   /* c   */ "+C",
			/* d   */ "+D",   /* e   */ "+E",   /* f   */ "+F",   /* g   */ "+G",
			/* h   */ "+H",   /* i   */ "+I",   /* j   */ "+J",   /* k   */ "+K",
			/* l   */ "+L",   /* m   */ "+M",   /* n   */ "+N",   /* o   */ "+O",
			/* p   */ "+P",   /* q   */ "+Q",   /* r   */ "+R",   /* s   */ "+S",
			/* t   */ "+T",   /* u   */ "+U",   /* v   */ "+V",   /* w   */ "+W",
			/* x   */ "+X",   /* y   */ "+Y",   /* z   */ "+Z",   /* {   */ "%P",
			/* |   */ "%Q",   /* }   */ "%R",   /* ~   */ "%S",   /* DEL */ "%T" 
		};


		protected override bool validate( string data )
		{
			for ( int i=0; i < data.length; i++ )
			{
				if ( (data[i] < 0) || (data[i] > 0x7F) )
				{
					return false;
				}
			}

			return true;
		}


		protected override string preprocess( string data )
		{
			StringBuilder extended_data = new StringBuilder("");

			for ( int i=0; i < data.length; i++ )
			{
				extended_data.append( ascii_map[ data[i] ] );
			}
				
			return extended_data.str;
		}


	}

}
