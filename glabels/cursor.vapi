namespace glabels
{

	[CCode (prefix = "", lower_case_cprefix = "cursor_", cheader_filename = "cursors/cursor_pixdata.h")]
	namespace Cursor
	{

		public Gdk.Pixdata barcode_pixdata;
		public Gdk.Pixdata box_pixdata;
		public Gdk.Pixdata ellipse_pixdata;
		public Gdk.Pixdata image_pixdata;
		public Gdk.Pixdata line_pixdata;
		public Gdk.Pixdata text_pixdata;

	}

}
