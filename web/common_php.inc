<?php
  header("Content-Type: text/html; charser=utf-8");

  #-------------------------------------------------------
  # Globals
  #-------------------------------------------------------
  # Complete real path to this file
  $file = realpath(__FILE__);

  # Complete real path of glabels root directory
  $root_dir = dirname($file);

  # Server document root
  $htroot = getenv("DOCUMENT_ROOT");

  # Base URL of the glabels website
  if ( strncmp($root_dir,$htroot,strlen($htroot)) == 0 ) {
    $root = substr($root_dir,strlen($htroot));
  } else {
    $root = "";
  }

  #-------------------------------------------------------
  # Print globals
  #-------------------------------------------------------
  function print_globals()
  {
    global $file, $root_dir, $htroot, $root;

    print "file = \"$file\"<br>";
    print "root_dir = \"$root_dir\"<br>";
    print "htroot = \"$htroot\"<br>";
    print "root = \"$root\"<br>";
  }

  #-------------------------------------------------------
  # Print root URL
  #-------------------------------------------------------
  function root()
  {
    global $root;

    print $root;
  }

  #-------------------------------------------------------
  # Construct and print img tag
  #-------------------------------------------------------
  function img($src, $alt)
  {
    global $root;
    global $root_dir;

    if (substr($src,0,1) == "/") {
      $size = GetImageSize("$root_dir$src");
      print "<img src=\"$root$src\" $size[3] alt=\"$alt\" />";
    } else {
      $size = GetImageSize("$src");
      print "<img src=\"$src\" $size[3] alt=\"$alt\" />";
    }
  }

  #-------------------------------------------------------
  # Construct and print img tag with explicit size, NULL alt
  #-------------------------------------------------------
  function img_size($src, $w, $h)
  {
    global $root;

    $alt = "";

    if (substr($src,0,1) == "/") {
      print "<img src=\"$root$src\" width=\"$w\" height=\"$h\" alt=\"$alt\" />";
    } else {
      print "<img src=\"$src\" width=\"$w\" height=\"$h\" alt=\"$alt\" />";
    }
  }

  #-------------------------------------------------------
  # Construct and print link (a) tag
  #-------------------------------------------------------
  function a($href, $text)
  {
    global $root;

    if (substr($href,0,1) == "/") {
      print "<a href=\"$root$href\">$text</a>";
    } else {
      print "<a href=\"$href\">$text</a>";
    }
  }

  #-------------------------------------------------------
  # Construct and print link (a) tag to a downloadable file
  #-------------------------------------------------------
  function a_file($href)
  {
    global $root;

    $text = basename($href);

    if (substr($href,0,1) == "/") {
      print "<a href=\"$root$href\">$text</a>";
    } else {
      print "<a href=\"$href\">$text</a>";
    }
  }

  #-------------------------------------------------------
  # Construct and print link (a) tag to a sf dl file
  #-------------------------------------------------------
  function a_sfdownload($file)
  {
      print "<a href=\"http://downloads.sourceforge.net/glabels/$file\">$file</a>";
  }

  #-------------------------------------------------------
  # Construct and print link (a) tag to a web address
  #-------------------------------------------------------
  function a_host($host)
  {
    global $root;

    print "<a href=\"http://$host/\">$host</a>";
  }

  #-------------------------------------------------------
  # Construct and print mailto (a) tag
  #-------------------------------------------------------
  function a_mailto($email)
  {
      print "<a href=\"mailto:$email\">$email</a>";
  }

  #-------------------------------------------------------
  # Common Header and navigation
  #-------------------------------------------------------
  function gLabelsHeader($tab)
  {
    global $root;

    $tabs["$root/"]             = "home";
    $tabs["$root/news/"]        = "news";
    $tabs["$root/screenshots/"] = "screenshots";
    $tabs["$root/download/"]    = "download";
    $tabs["$root/doc/"]         = "docs";
    $tabs["$root/faq/"]         = "FAQ";
    $tabs["$root/contact/"]     = "contact";
    $tabs["$root/links/"]       = "links";

    print "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN"
    "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
	 <meta name="description" content="gLabels: a label and business card program for GNOME" />
	 <meta name="keywords" content="glabels, snaught, label, business card, gnome, linux" />

	 <title>gLabels</title>

	 <link rel="stylesheet" href="<?root();?>/css/glabels.css" type="text/css" />

</head>

<body id="glabels">

	<div id="title">&nbsp;</div>

	<div id="left">

		<!-- Navigation tabs -->
		<div id="nav">
			<ul>
<?php
    #
    # Generate tabs
    #
    $key = "";
    $value = "";
    while (list($key,$value) = each($tabs))
    {
        $value = trim($value);
	if (strlen($value) >= 8) {$width=90;} else {$width=60;}
        if ($value == $tab)
        {
                print "\t\t\t<li><span>$tab</span></li>\n";
        }
        else
        {
                print "\t\t\t<li><a href=\"$key\">$value</a></li>\n";
        }
    }
?>

		 	</ul>

		</div>

		<div id="sflink">
			<a href="http://sourceforge.net"><img class="noborder" src="http://sourceforge.net/sflogo.php?group_id=46122&amp;type=2" width="125" height="37" alt="SourceForge Logo" /></a>
		</div>

	</div>

	<div id="main">
        	<div class="tlcorner">&nbsp;</div>
		<div class="trcorner">&nbsp;</div>
		<div id="content">


<?php
}
?>


<?php

  #-------------------------------------------------------
  # Common Footer
  #-------------------------------------------------------
  function gLabelsFooter()
  {
?>


		</div>
		<div class="blcorner">&nbsp;</div>
		<div class="brcorner">&nbsp;</div>

		<div id="footer">
						      <div id="copy">* Section header icons on this website borrowed from the <a href="http://tango.freedesktop.org">Tango</a> project. (<a href="http://creativecommons.org/licenses/by-sa/2.5/">License</a>)</div>

			<div id="w3clinks">
                                <a href="http://validator.w3.org/check/referer"><img class="noborder" src="http://www.w3.org/Icons/valid-xhtml11" alt="Valid XHTML 1.1!" height="31" width="88" /></a>
				<a href="http://jigsaw.w3.org/css-validator/"><img class="noborder" src="http://jigsaw.w3.org/css-validator/images/vcss" alt="Valid CSS!" height="31" width="88" /></a>
			</div>

		</div>

	</div>

</body>

</html>

<?php
  }
?>


