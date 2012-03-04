/*  color_node.vala
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

	public struct ColorNode
	{
		public bool    field_flag { get; set; }
		public Color   color      { get; set; }
		public string? key        { get; set; }


		public ColorNode( bool field_flag, Color color, string? key )
		{
			this.field_flag = field_flag;
			this.color      = color;
			this.key        = key;
		}


		public ColorNode.from_color( Color color )
		{
			this( false, color, null );
		}


		public bool equal( ColorNode cn )
		{
			return ( (this.field_flag == cn.field_flag) && this.color.equal( cn.color ) && (this.key == cn.key) );
		}


		public Color expand( MergeRecord? record )
		{
			if ( field_flag )
			{
				if ( record == null )
				{
					return Color.none();
				}
				else
				{
					string? text = record.eval_key( key );
					if ( text != null )
					{
						Gdk.Color gdk_color = Gdk.Color();
						if ( Gdk.Color.parse( text, out gdk_color ) )
						{
							Color color = Color.from_gdk_color( gdk_color );
							return color;
						}
						else
						{
							return Color.none();
						}
					}
					else
					{
						return Color.none();
					}
				}
			}
			else
			{
				return color;
			}
		}


	}

}
