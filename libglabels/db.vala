/*  db.vala
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

	public class Db : Object
	{

		public signal void changed();

		private class StaticChangedProxy
		{
			internal signal void changed();

			internal void emit_changed()
			{
				changed();
			}
		}

		private static StaticChangedProxy static_changed_proxy;

		public static unowned List<Paper?>    papers         { get; private set; }
		public static unowned List<string>    paper_ids      { get; private set; }
		public static unowned List<string>    paper_names    { get; private set; }

		public static unowned List<Category?> categories     { get; private set; }
		public static unowned List<string>    category_ids   { get; private set; }
		public static unowned List<string>    category_names { get; private set; }

		public static unowned List<Vendor?>   vendors        { get; private set; }
		public static unowned List<string>    vendor_names   { get; private set; }

		public static unowned List<Template>  templates      { get; private set; }


		public static void init()
		{
			/* Force construction of static fields. */
			new Db();
		}


		static construct
		{
			static_changed_proxy = new StaticChangedProxy();

			papers         = null;
			paper_ids      = null;
			paper_names    = null;

			categories     = null;
			category_ids   = null;
			category_names = null;

			vendors        = null;
			vendor_names   = null;

			templates      = null;

			/*
			 * Paper definitions
			 */
			read_papers();

			/* Create and append an "Other" entry. */
			/* Translators: "Other" here means other page size.  Meaning a page size
			 * other than the standard ones that libglabels knows about such as
			 * "letter", "A4", etc. */
			Paper paper_other = Paper( "Other", _("Other"), 0.0, 0.0, null );
			register_paper( paper_other );

			/*
			 * Category definitions
			 */
			read_categories();

			/* Create and append a "User defined" entry. */
			Category category_user = Category( "user-defined", _("User defined") );
			register_category( category_user );

			/*
			 * Vendor definitions
			 */
			read_vendors();

			/*
			 * Templates
			 */
			read_templates();

			/* Create and append generic full page templates. */
			foreach ( string paper_id in paper_ids )
			{
				if ( !is_paper_id_other( paper_id ) )
				{
					Template template = new Template.full_page( paper_id );
					register_template( template );
				}
			}
		}


		construct
		{
			static_changed_proxy.changed.connect( on_proxy_changed );
		}


		private void on_proxy_changed()
		{
			changed();
		}


		internal static void register_paper( Paper paper )
		{
			if ( lookup_paper_from_id( paper.id ) == null )
			{
				papers.append( paper );
				paper_ids.append( paper.id );
				paper_names.append( paper.name );
			}
			else
			{
				message( "Duplicate paper id: %s.", paper.id );
			}
		}


		public static Paper? lookup_paper_from_name( string? name )
		{
			if ( name == null )
			{
				return papers.first().data;
			}

			foreach ( Paper paper in papers )
			{
				if ( paper.name == name )
				{
					return paper;
				}
			}

			return null;
		}


		public static Paper? lookup_paper_from_id( string? id )
		{
			if ( id == null )
			{
				return papers.first().data;
			}

			foreach ( Paper paper in papers )
			{
				if ( paper.id == id )
				{
					return paper;
				}
			}

			return null;
		}


		public static string? lookup_paper_id_from_name( string? name )
		{
			if ( name != null )
			{
				Paper paper = lookup_paper_from_name( name );
				return paper.id;
			}

			return null;
		}


		public static string? lookup_paper_name_from_id( string? id )
		{
			if ( id != null )
			{
				Paper paper = lookup_paper_from_id( id );
				return paper.name;
			}

			return null;
		}


		public static bool is_paper_id_known( string? id )
		{
			if ( id == null )
			{
				return false;
			}

			foreach (Paper paper in papers)
			{
				if ( paper.id == id )
				{
					return true;
				}
			}

			return false;
		}


		public static bool is_paper_id_other( string? id )
		{
			return ( id == "Other" );
		}


		internal static void register_category( Category category )
		{
			if ( lookup_category_from_id( category.id ) == null )
			{
				categories.append( category );
				category_ids.append( category.id );
				category_names.append( category.name );
			}
			else
			{
				message( "Duplicate category id: %s.", category.id );
			}
		}


		public static Category? lookup_category_from_name( string? name )
		{
			if ( name == null )
			{
				return categories.first().data;
			}

			foreach ( Category category in categories )
			{
				if ( category.name == name )
				{
					return category;
				}
			}

			return null;
		}


		public static Category? lookup_category_from_id( string? id )
		{
			if ( id == null )
			{
				return categories.first().data;
			}

			foreach ( Category category in categories )
			{
				if ( category.id == id )
				{
					return category;
				}
			}

			return null;
		}


		public static string? lookup_category_id_from_name( string? name )
		{
			if ( name != null )
			{
				Category category = lookup_category_from_name( name );
				return category.id;
			}

			return null;
		}


		public static string? lookup_category_name_from_id( string? id )
		{
			if ( id != null )
			{
				Category category = lookup_category_from_id( id );
				return category.name;
			}

			return null;
		}


		public static bool is_category_id_known( string? id )
		{
			if ( id == null )
			{
				return false;
			}

			foreach (Category category in categories)
			{
				if ( category.id == id )
				{
					return true;
				}
			}

			return false;
		}


		internal static void register_vendor( Vendor vendor )
		{
			if ( lookup_vendor_from_name( vendor.name ) == null )
			{
				vendors.append( vendor );
				vendor_names.append( vendor.name );
			}
			else
			{
				message( "Duplicate vendor name: %s.", vendor.name );
			}
		}


		public static Vendor? lookup_vendor_from_name( string? name )
		{
			if ( name == null )
			{
				return vendors.first().data;
			}

			foreach ( Vendor vendor in vendors )
			{
				if ( vendor.name == name )
				{
					return vendor;
				}
			}

			return null;
		}


		public static string? lookup_url_from_name( string? name )
		{
			if ( name != null )
			{
				Vendor vendor = lookup_vendor_from_name( name );
				return vendor.url;
			}

			return null;
		}


		public static bool is_vendor_name_known( string? name )
		{
			if ( name == null )
			{
				return false;
			}

			foreach (Vendor vendor in vendors)
			{
				if ( vendor.name == name )
				{
					return true;
				}
			}

			return false;
		}


		internal static void register_template( Template template )
		{
			if ( !does_template_exist( template.brand, template.part ) )
			{
				templates.insert_sorted( template, compare_template_names );
			}
			else
			{
				message( "Duplicate template: %s %s.",
				         template.brand, template.part );
			}
		}


		public static void register_user_template( Template template )
		{
			return_if_fail( !does_template_exist( template.brand, template.part ) );
			return_if_fail( is_paper_id_known( template.paper_id ) );

			string dir = Path.build_filename( Environment.get_user_config_dir(),
			                                  "libglabels", "templates",
			                                  null );
			DirUtils.create_with_parents( dir, 0775 ); /* Make sure dir exists. */

			string filename = "%s_%s.template".printf( template.brand, template.part );
			string abs_filename = Path.build_filename( dir, filename, null );

			int bytes_written = XmlTemplate.write_template_to_file( template, abs_filename );

			return_if_fail( bytes_written > 0 );

			Template template_copy = template.dup();
			template_copy.add_category( "user-defined" );
			register_template( template_copy );

			static_changed_proxy.emit_changed();
		}


		public static void delete_user_template_by_name( string name )
		{
			Template template = lookup_template_from_name( name );

			return_if_fail( template != null );
			return_if_fail( template.does_category_match( "user-defined" ) );

			string dir = Path.build_filename( Environment.get_user_config_dir(),
			                                  "libglabels", "templates",
			                                  null );

			string filename = "%s_%s.template".printf( template.brand, template.part );
			string abs_filename = Path.build_filename( dir, filename, null );
			return_if_fail( FileUtils.test( abs_filename, FileTest.EXISTS ) );

			FileUtils.unlink( abs_filename );

			templates.remove( template );
		}


		public static void delete_user_template_by_brand_part( string brand,
		                                                       string part )
		{
			string name = "%s %s".printf( brand, part );

			delete_user_template_by_name( name );
		}


		public static Template? lookup_template_from_name( string? name )
		{
			if ( name == null )
			{
				return templates.first().data;
			}

			foreach ( Template template in templates )
			{
				if ( template.name == name )
				{
					return template;
				}
			}

			return null;
		}


		public static Template? lookup_template_from_brand_part( string? brand,
		                                                         string? part )
		{
			if ( (brand == null) || (part == null) )
			{
				return templates.first().data;
			}

			foreach ( Template template in templates )
			{
				if ( (template.brand == brand) && (template.part == part) )
				{
					return template;
				}
			}

			return null;
		}


		public static bool does_template_exist( string? brand,
		                                        string? part )
		{
			return ( lookup_template_from_brand_part( brand, part ) != null );
		}


		/************************************/
		/* Debug methods.                   */
		/************************************/

		public static void print_known_papers()
		{
			stdout.printf( "KNOWN PAPERS:\n" );

			foreach (Paper paper in papers)
			{
				stdout.printf( "paper id=\"%s\", name=\"%s\" width=%gpts, height=%gpts\n",
				               paper.id,
				               paper.name,
				               paper.width,
				               paper.height );
			}

			stdout.printf( "\n" );
		}


		public static void print_known_categories()
		{
			stdout.printf( "KNOWN CATEGORIES:\n" );

			foreach (Category category in categories)
			{
				stdout.printf( "category id=\"%s\", name=\"%s\"\n",
				               category.id,
				               category.name );
			}

			stdout.printf( "\n" );
		}


		public static void print_known_vendors()
		{
			stdout.printf( "KNOWN VENDORS:\n" );

			foreach (Vendor vendor in vendors)
			{
				stdout.printf( "vendor name=\"%s\", url=\"%s\"\n",
				               vendor.name,
				               vendor.url );
			}

			stdout.printf( "\n" );
		}


		public static void print_known_templates()
		{
			stdout.printf( "KNOWN TEMPLATES:\n" );

			foreach (Template template in templates)
			{
				stdout.printf( "template brand=\"%s\", part=\"%s\", description=\"%s\"\n",
				               template.brand,
				               template.part,
				               template.description );
			}

			stdout.printf( "\n" );
		}


		/************************************/
		/* Methods to initialize db.        */
		/************************************/

		private static void read_papers()
		{
			string data_dir;

			data_dir = Path.build_filename( Config.DATADIR, Config.LIBGLABELS_BRANCH, "templates", null );
			read_paper_files_from_dir( data_dir );

			data_dir = Path.build_filename( Environment.get_user_config_dir(), "libglabels", "templates", null );
			read_paper_files_from_dir( data_dir );

			data_dir = Path.build_filename( Environment.get_home_dir(), ".glabels", null );
			read_paper_files_from_dir( data_dir );

			if ( papers == null )
			{
				critical( "Unable to locate paper size definitions. Libglabels may not be installed correctly!" );
			}
		}


		private static void read_paper_files_from_dir( string dirname )
		{
			if ( FileUtils.test( dirname, FileTest.IS_DIR ) )
			{
				Dir dir;

				try {
					dir = Dir.open( dirname, 0 );
				}
				catch( Error e )
				{
					message( "cannot open data directory: %s", e.message );
					return;
				}

				string? filename;

				while ( (filename = dir.read_name()) != null )
				{
					if ( filename == "paper-sizes.xml" )
					{
						string full_filename = Path.build_filename( dirname, filename, null );
						XmlPaper.read_papers_from_file( full_filename );
					}
				}

			}
		}


		private static void read_categories()
		{
			string data_dir;

			data_dir = Path.build_filename( Config.DATADIR, Config.LIBGLABELS_BRANCH, "templates", null );
			read_category_files_from_dir( data_dir );

			data_dir = Path.build_filename( Environment.get_user_config_dir(), "libglabels", "templates", null );
			read_category_files_from_dir( data_dir );

			data_dir = Path.build_filename( Environment.get_home_dir(), ".glabels", null );
			read_category_files_from_dir( data_dir );

			if ( categories == null )
			{
				critical( "Unable to locate any category definitions. Libglabels may not be installed correctly!" );
			}
		}


		private static void read_category_files_from_dir( string dirname )
		{
			if ( FileUtils.test( dirname, FileTest.IS_DIR ) )
			{
				Dir dir;

				try {
					dir = Dir.open( dirname, 0 );
				}
				catch( Error e )
				{
					message( "cannot open data directory: %s", e.message );
					return;
				}

				string? filename;

				while ( (filename = dir.read_name()) != null )
				{
					if ( filename == "categories.xml" )
					{
						string full_filename = Path.build_filename( dirname, filename, null );
						XmlCategory.read_categories_from_file( full_filename );
					}
				}

			}
		}


		private static void read_vendors()
		{
			string data_dir;

			data_dir = Path.build_filename( Config.DATADIR, Config.LIBGLABELS_BRANCH, "templates", null );
			read_vendor_files_from_dir( data_dir );

			data_dir = Path.build_filename( Environment.get_user_config_dir(), "libglabels", "templates", null );
			read_vendor_files_from_dir( data_dir );

			data_dir = Path.build_filename( Environment.get_home_dir(), ".glabels", null );
			read_vendor_files_from_dir( data_dir );

			if ( vendors == null )
			{
				critical( "Unable to locate any vendor definitions. Libglabels may not be installed correctly!" );
			}
		}


		private static void read_vendor_files_from_dir( string dirname )
		{
			if ( FileUtils.test( dirname, FileTest.IS_DIR ) )
			{
				Dir dir;

				try {
					dir = Dir.open( dirname, 0 );
				}
				catch( Error e )
				{
					message( "cannot open data directory: %s", e.message );
					return;
				}

				string? filename;

				while ( (filename = dir.read_name()) != null )
				{
					if ( filename == "vendors.xml" )
					{
						string full_filename = Path.build_filename( dirname, filename, null );
						XmlVendor.read_vendors_from_file( full_filename );
					}
				}

			}
		}


		private static void read_templates()
		{
			string data_dir;

			data_dir = Path.build_filename( Config.DATADIR, Config.LIBGLABELS_BRANCH, "templates", null );
			read_template_files_from_dir( data_dir );

			data_dir = Path.build_filename( Environment.get_user_config_dir(), "libglabels", "templates", null );
			read_template_files_from_dir( data_dir );

			data_dir = Path.build_filename( Environment.get_home_dir(), ".glabels", null );
			read_template_files_from_dir( data_dir );

			if ( templates == null )
			{
				critical( "Unable to locate any template definitions. Libglabels may not be installed correctly!" );
			}
		}


		private static void read_template_files_from_dir( string dirname )
		{
			if ( FileUtils.test( dirname, FileTest.IS_DIR ) )
			{
				Dir dir;

				try {
					dir = Dir.open( dirname, 0 );
				}
				catch( Error e )
				{
					message( "cannot open data directory: %s", e.message );
					return;
				}

				string? filename;

				while ( (filename = dir.read_name()) != null )
				{
					if ( filename.has_suffix( "-templates.xml" ) ||
						 filename.has_suffix( ".template" ) )
					{
						string full_filename = Path.build_filename( dirname, filename, null );
						XmlTemplate.read_templates_from_file( full_filename );
					}
				}

			}
		}


	}

}
