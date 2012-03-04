/*  enum_util.vala
 *
 *  Copyright (C) 2011  Jim Evins <evins@snaught.com>
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

	namespace EnumUtil
	{

		/****************************************************************************/
		/* Utilities to deal with PangoAlignment types.                             */
		/****************************************************************************/

		public string align_to_string( Pango.Alignment align )
		{
			switch (align) {
			case Pango.Alignment.LEFT:
				return "left";
			case Pango.Alignment.CENTER:
				return "center";
			case Pango.Alignment.RIGHT:
				return "right";
			default:
				return "?";
			}
		}

		public Pango.Alignment string_to_align( string align_string )
		{
			switch (align_string)
			{
			case "left":
			case "Left":
			case "LEFT":
				return Pango.Alignment.LEFT;
			case "center":
			case "Center":
			case "CENTER":
				return Pango.Alignment.CENTER;
			case "right":
			case "Right":
			case "RIGHT":
				return Pango.Alignment.RIGHT;
			default:
				return Pango.Alignment.LEFT;
			}
		}


		/****************************************************************************/
		/* Utilities to deal with PangoWeight types                                 */
		/****************************************************************************/

		public string weight_to_string( Pango.Weight weight )
		{
			switch (weight) {
			case Pango.Weight.NORMAL:
				return "regular";
			case Pango.Weight.BOLD:
				return "bold";
			default:
				return "?";
			}
		}

		public Pango.Weight string_to_weight( string weight_string )
		{
			switch (weight_string)
			{
			case "regular":
			case "Regular":
			case "REGULAR":
                return Pango.Weight.NORMAL;
			case "bold":
			case "Bold":
			case "BOLD":
                return Pango.Weight.BOLD;
			default:
                return Pango.Weight.NORMAL;
			}
		}


	}

}
