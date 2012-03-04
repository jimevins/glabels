
using glabels;

int main (string[] args) {

    Gtk.init( ref args );

    libglabels.Db.init();
    libglabels.XmlUtil.init();

    var win = new Gtk.Window();
    win.set_size_request( 200, 200 );
    win.border_width = 5;
    win.title = "Widget test";
    win.destroy.connect( Gtk.main_quit );

    var frame = new Gtk.Frame( "Example Vala Widget" );
    win.add( frame );

	var vbox = new Gtk.VBox( false, 3 );
	frame.add( vbox );


	switch (args[1])
	{

	case "0":
		stdout.printf( "TEST %s.\n", args[1] );
        libglabels.Db.print_known_papers();
        libglabels.Db.print_known_categories();
        libglabels.Db.print_known_vendors();
        libglabels.Db.print_known_templates();
        break;

	case "1":
		stdout.printf( "TEST %s.\n", args[1] );
		var color = Color.from_rgb( 0.75, 0.75, 1.0 );
		var button1 = new glabels.ColorButton( "Xyzzy", color, color );
		vbox.pack_start( button1 );

		var button2 = new glabels.FontButton( "Sans" );
		vbox.pack_start( button2 );
		break;

	case "2":
		stdout.printf( "TEST %s.\n", args[1] );
		var mini_preview = new glabels.MiniPreview( 200, 200 );
		mini_preview.set_template_by_name( "Avery 3612" );
		vbox.pack_start( mini_preview );
		break;

	case "3":
		stdout.printf( "TEST %s.\n", args[1] );
		Gtk.ListStore list_store = new Gtk.ListStore( 2, typeof(string), typeof(Gdk.Pixbuf) );

		var icon_view = new Gtk.IconView.with_model( list_store );
		icon_view.set_text_column( 0 );
		icon_view.set_pixbuf_column( 1 );

		var sw = new Gtk.ScrolledWindow( null, null );
		sw.add( icon_view );
		vbox.pack_start( sw );

		foreach ( libglabels.Template template in libglabels.Db.templates )
		{
			Gtk.TreeIter iter;
			list_store.append( out iter );

			list_store.set( iter, 0, template.name, 1, template.preview_pixbuf, -1);
		}
		break;
	
	case "4":
		stdout.printf( "TEST %s.\n", args[1] );
		var label = new glabels.Label();
		label.template = libglabels.Db.lookup_template_from_name( "Avery 3612" );

		var box = new LabelObjectBox();
		box.x0 = 36;
		box.y0 = 36;
		box.w  = 72;
		box.h  = 36;
		box.line_width = 4;
		box.line_color_node = ColorNode.from_color( Color.black() );
		box.fill_color_node = ColorNode.from_color( Color.from_rgb( 0, 1, 0 ) );

		label.add_object( box );

		var mini_preview = new glabels.MiniPreview( 200, 200 );
		mini_preview.set_label( label );
		vbox.pack_start( mini_preview );
		break;

	case "4.1":
		stdout.printf( "TEST %s.\n", args[1] );
		var label = new glabels.Label();
		label.template = libglabels.Db.lookup_template_from_name( "Avery 3612" );
		label.rotate = true;

		var box = new LabelObjectBox();
		box.x0 = 36;
		box.y0 = 36;
		box.w  = 72;
		box.h  = 36;
		box.line_width = 4;
		box.line_color_node = ColorNode.from_color( Color.black() );
		box.fill_color_node = ColorNode.from_color( Color.from_rgb( 0, 1, 0 ) );

		label.add_object( box );

		var mini_preview = new glabels.MiniPreview( 200, 200 );
		mini_preview.set_label( label );
		vbox.pack_start( mini_preview );
		break;

	case "5":
		stdout.printf( "TEST %s.\n", args[1] );
		var label = new glabels.Label();
		label.template = libglabels.Db.lookup_template_from_name( "Avery 5523" );

		var box = new LabelObjectBox();
		box.x0 = 36;
		box.y0 = 36;
		box.w  = 72;
		box.h  = 36;
		box.line_width = 2;
		box.line_color_node = ColorNode.from_color( Color.black() );
		box.fill_color_node = ColorNode.from_color( Color.from_rgb( 0, 1, 0 ) );

		label.add_object( box );

		var view = new glabels.View( label );
		vbox.pack_start( view );
		break;

	case "5.1":
		stdout.printf( "TEST %s.\n", args[1] );
		var label = new glabels.Label();
		label.template = libglabels.Db.lookup_template_from_name( "Avery 5523" );
		label.rotate = true;

		var box = new LabelObjectBox();
		box.x0 = 36;
		box.y0 = 36;
		box.w  = 72;
		box.h  = 36;
		box.line_width = 2;
		box.line_color_node = ColorNode.from_color( Color.black() );
		box.fill_color_node = ColorNode.from_color( Color.from_rgb( 0, 1, 0 ) );

		label.add_object( box );

		var view = new glabels.View( label );
		vbox.pack_start( view );
		break;

	case "6":
		stdout.printf( "TEST %s.\n", args[1] );
		Merge merge = MergeFactory.create_merge( "Text/Comma" );
		merge.src = "./UH-UTF8.csv";
		foreach ( MergeRecord record in merge.record_list )
		{
			stdout.printf( "RECORD:\n" );
			foreach ( MergeField field in record.field_list )
			{
				stdout.printf( "[ %s ] = %s\n", field.key, field.value );
			}
			stdout.printf( "\n" );
		}
		break;

	case "6.1":
		stdout.printf( "TEST %s.\n", args[1] );
		Merge merge = MergeFactory.create_merge( "Text/Comma/Line1Keys" );
		merge.src = "./UH-UTF8.csv";
		foreach ( MergeRecord record in merge.record_list )
		{
			stdout.printf( "RECORD:\n" );
			foreach ( MergeField field in record.field_list )
			{
				stdout.printf( "[ %s ] = %s\n", field.key, field.value );
			}
			stdout.printf( "\n" );
		}
		break;

	case "7":
		stdout.printf( "TEST %s.\n", args[1] );
		glabels.Label label = XmlLabel.open_file( "test1.glabels" );
		var view = new glabels.View( label );
		vbox.pack_start( view );
		break;

	case "8":
		stdout.printf( "TEST %s.\n", args[1] );
		libglabels.XmlUtil.default_units = libglabels.Units.inch();
		glabels.Label label = XmlLabel.open_file( "test1.glabels" );
		XmlLabel.save_file( label, "test1-out.glabels" );
		break;

	case "9":
		stdout.printf( "TEST %s.\n", args[1] );
		Gtk.IconTheme.get_default().append_search_path( Path.build_filename( Config.DATADIR, "glabels-3.0", "icons", null ) );
		libglabels.XmlUtil.default_units = libglabels.Units.inch();
		glabels.Label label = XmlLabel.open_file( "test1.glabels" );
		var window = new Window.from_label( label );
		window.show_all();
		break;

	default:
		stdout.printf( "NO TEST SELECTED.\n" );
		break;
	}


    win.show_all();
    Gtk.main();

    return 0;
}

