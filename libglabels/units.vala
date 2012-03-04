/*  units.vala
 *
 *  Copyright (C) 2011  Jim Evins <evins@snaught.com>
 *
 *  This file is part of libglabels.
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

namespace libglabels
{

	private const double POINTS_PER_POINT =  1.0; /* internal units are points. */
	private const double POINTS_PER_INCH  = 72.0;
	private const double POINTS_PER_MM    =  2.83464566929;
	private const double POINTS_PER_CM    =  (10.0*POINTS_PER_MM);
	private const double POINTS_PER_PICA  =  (1.0/12.0);


	public struct Units
	{
		public string id              { get; protected set; }
		public string name            { get; protected set; }
		public double points_per_unit { get; protected set; }

		public double units_per_point { get{ return 1.0 / points_per_unit; } }


		private Units( string id,
		               string name,
		               double points_per_unit )
		{
			this.id              = id;
			this.name            = name;
			this.points_per_unit = points_per_unit;
		}


		public Units.from_id( string id )
		{
			string name;
			double points_per_unit;

			switch (id)
			{

			case "pt":
				name            = _("points");
				points_per_unit = POINTS_PER_POINT;
				break;

			case "in":
				name            = _("inches");
				points_per_unit = POINTS_PER_INCH;
				break;

			case "mm":
				name            = _("mm");
				points_per_unit = POINTS_PER_MM;
				break;

			case "cm":
				name            = _("cm");
				points_per_unit = POINTS_PER_CM;
				break;

			case "pc":
				name            = _("picas");
				points_per_unit = POINTS_PER_PICA;
				break;

			case "":
				/* Missing or empty units id defaults to points. */
				id              = "pt";
				name            = _("points");
				points_per_unit = POINTS_PER_POINT;
				break;

			default:
				warning( "Unknown Units.id \"%s\"", id );
				id              = "pt";
				name            = _("points");
				points_per_unit = POINTS_PER_POINT;
				break;

			}

			this( id, name, points_per_unit );
		}

		public Units.point()
		{
			this( "pt", _("points"), POINTS_PER_POINT );
		}


		public Units.inch()
		{
			this( "in", _("inches"), POINTS_PER_INCH );
		}


		public Units.mm()
		{
			this( "mm", _("mm"), POINTS_PER_MM );
		}


		public Units.cm()
		{
			this( "cm", _("cm"), POINTS_PER_CM );
		}


		public Units.pica()
		{
			this( "pc", _("picas"), POINTS_PER_PICA );
		}


	}

}
