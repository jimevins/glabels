/*  units_util.vala
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
using libglabels;

namespace glabels
{

	namespace UnitsUtil
	{

		/**
		 * Get step size for desired units.
		 */
		public double get_step_size( Units units )
		{

			switch (units.id)
			{
			case "pt":
                return 0.1;     /* points */
			case "in":
                return 0.001;   /* inches */
			case "mm":
                return 0.1;     /* mm */
			default:
                warning( "Illegal units" );    /* Should not happen */
                return 1.0;
			}
		}


		/**
		 * Get precision for desired units.
		 */
		public int get_precision( Units units )
		{

			switch (units.id)
			{
			case "pt":
                return 1;       /* points */
			case "in":
                return 3;       /* inches */
			case "mm":
                return 1;       /* mm */
			default:
                warning( "Illegal units" );    /* Should not happen */
                return 1;
			}
		}


		/**
		 * Get grid size for desired units.
		 */
		public double get_grid_size( Units units )
		{

			switch (units.id)
			{
			case "pt":
				return 10.0;
			case "in":
				return 0.125 * units.points_per_unit;
			case "mm":
				return 5     * units.points_per_unit;
			default:
				warning( "Illegal units" );    /* Should not happen */
				return 10;
			}
		}


	}

}
