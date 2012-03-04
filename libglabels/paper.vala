/*  paper.vala
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

	public struct Paper
	{
		public string   id       { get; private set; }   /* Unique ID of category */
		public string   name     { get; private set; }   /* Localized name of category */
		public double   width    { get; private set; }   /* Width (in points) */
		public double   height   { get; private set; }   /* Width (in points) */
		public string   pwg_size { get; private set; }   /* PWG 5101.1-2002 size name */

		public Paper( string  id,
		              string  name,
		              double  width,
		              double  height,
		              string? pwg_size )
		{
			this.id       = id;
			this.name     = name;
			this.width    = width;
			this.height   = height;
			this.pwg_size = pwg_size;
		}

	}

}

