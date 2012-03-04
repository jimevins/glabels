/*  file_util.vala
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

	namespace FileUtil
	{

		public string make_absolute( string filename )
		{
			if ( Path.is_absolute( filename ) )
			{
				return filename;
			}
			else
			{
				string pwd = Environment.get_current_dir();
				return Path.build_filename( pwd, filename, null );
			}
		}


		public string add_extension( string filename )
		{
			if ( filename.has_suffix( ".glabels" ) )
			{
				return filename;
			}
			else
			{
				return filename.concat( ".glabels", null );
			}
		}

	}

}
