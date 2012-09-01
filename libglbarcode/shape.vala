/*  shape.vala
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

	public abstract class Shape
	{
		public double x;
		public double y;
	}

	/**
	 * A solid box drawing primitive.
	 *
	 *<programlisting>
	 *
	 * @ =  origin (x,y) from top left corner of barcode
	 *
	 *              @---------+
	 *              |         |
	 *              |         |
	 *              |         |
	 *              |         | h
	 *              |         |
	 *              |         |
	 *              |         |
	 *              +---------+
	 *                   w
	 *
	 *</programlisting>
	 *
	 * All units are in points ( 1 point = 1/72 inch ).
	 */
	public class ShapeBox : Shape
	{
		public double              w;
		public double              h;

		public ShapeBox( double x, double y, double w, double h )
		{
			this.x = x;
			this.y = y;
			this.w = w;
			this.h = h;
		}
	}


	/**
	 * A character string drawing primitive.
	 *
	 *<programlisting>
	 *
	 * @ =  origin (x,y) from top left corner of barcode
	 *
	 *              ____        _  ------------------
	 *             /    \      | |                  ^
	 *            /  /\  \     | |                  |
	 *           /  /__\  \    | |___     ____      |
	 *          /  ______  \   | ._  \   /  __|     | ~fsize
	 *         /  /      \  \  | |_)  | |  (__      |
	 *        /__/        \__\ |_.___/   \____|     |
	 *                                              v
	 *                           @ ------------------
	 *                           x = horizontal center
	 *
	 *</programlisting>
	 *
	 * All units are in points ( 1 point = 1/72 inch ).
	 */
	public class ShapeText : Shape
	{
		public double              fsize;
		public string              s;

		public ShapeText( double x, double y, double fsize, string s )
		{
			this.x     = x;
			this.y     = y;
			this.fsize = fsize;
			this.s     = s;
		}
	}


	/**
	 * A ring (an open circle) drawing primitive.
	 *
	 *<programlisting>
	 *
	 * @ = origin (x,y) is centre of circle
	 *
	 *                v  line_width
	 *           _.-""""-._
	 *         .'   ____   `.
	 *        /   .'  ^ `.   \
	 *       |   /        \   |
	 *       |   |    @---|---|------
	 *       |   \        /   |     |
	 *        \   `.____.'   /      | r
	 *         `._    ...._.'.......v
	 *            `-....-'
	 *
	 *</programlisting>
	 *
	 * All units are in points ( 1 point = 1/72 inch ).
	 */
	public class ShapeRing : Shape
	{
		public double              r;
		public double              line_width;

		public ShapeRing( double x, double y, double r, double line_width )
		{
			this.x          = x;
			this.y          = y;
			this.r          = r;
			this.line_width = line_width;
		}
	}


	/**
	 * A solid regular hexagon (oriented with vertexes at top and bottom) drawing primitive.
	 *
	 *<programlisting>
	 *
	 * @ = origin (x,y) is top of hexagon
	 *
	 *                  @ ------------------
	 *              _-"   "-_              ^
	 *          _-"           "-_          |
	 *       +"                   "+       |
	 *       |                     |       |
	 *       |                     |       |
	 *       |                     |       | h
	 *       |                     |       |
	 *       |                     |       |
	 *       +_                   _+       |
	 *         "-_             _-"         |
	 *            "-_       _-"            |
	 *               "-_ _-"               v
	 *                  " ------------------
	 *
	 *</programlisting>
	 *
	 * All units are in points ( 1 point = 1/72 inch ).
	 */
	public class ShapeHexagon : Shape
	{
		public double              h;

		public ShapeHexagon( double x, double y, double h )
		{
			this.x = x;
			this.y = y;
			this.h = h;
		}
	}

}
