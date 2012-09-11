/*  barcode_style.vala
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

	public class BarcodeStyle
	{

		public string id                { get; private set; }
		public string name              { get; private set; }
		public bool   can_text          { get; private set; }
		public bool   text_optional     { get; private set; }
		public bool   can_checksum      { get; private set; }
		public bool   checksum_optional { get; private set; }
		public string default_digits    { get; private set; }
		public bool   can_freeform      { get; private set; }
		public int    prefered_n        { get; private set; }


		public BarcodeStyle ( string id,
		                      string name,
		                      bool   can_text,
		                      bool   text_optional,
		                      bool   can_checksum,
		                      bool   checksum_optional,
		                      string default_digits,
		                      bool   can_freeform,
		                      int    prefered_n )
		{
			this.id                = id;
			this.name              = name;
			this.can_text          = can_text;
			this.text_optional     = text_optional;
			this.can_checksum      = can_checksum;
			this.checksum_optional = checksum_optional;
			this.default_digits    = default_digits;
			this.can_freeform      = can_freeform;
			this.prefered_n        = prefered_n;
		}


		public string get_example_digits( int n )
		{
			if ( can_freeform )
			{
				return string.nfill( int.max(n,1), '0' );
			}
			else
			{
				return default_digits;
			}
		}


	}


}




