namespace Gdk {
	[CCode (cheader_filename = "gdk-pixbuf/gdk-pixdata.h", cprefix = "gdk_")]
	public void pixdata_from_pixbuf( out Gdk.Pixdata pixdata, Gdk.Pixbuf pixbuf, bool use_rle = false );
}
