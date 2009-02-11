<?php include "../common_php.inc"; ?>

<?php gLabelsHeader("FAQ"); ?>

<h1 class="doc">Frequently Asked Questions (FAQ) ...</h1>




<h2>Index</h2>
<div class="faqtoc">
<ul class="faqtoc">

  <li class="faqtocline">1. <?a("#S1", "Common Problems.");?>

  <ul class="faqtoc">

    <li class="faqtocline">1.1 <?a("#Q1.1","How can I instruct glabels to not print blank address lines when doing a document merge?");?></li>

  </ul>

  </li>

  <li class="faqtocline">2. <?a("#S2", "Obsolete Problems (problems with older versions of gLabels).");?>

  <ul class="faqtoc">

    <li class="faqtocline">2.1 <?a("#Q2.1","I cannot edit text.  What's wrong? (2.0.x)");?></li>
    <li class="faqtocline">2.2 <?a("#Q2.2","Why don't transparent images print correctly?");?></li>

  </ul>

  </li>

</ul>
</div>

<a id="S1"/>
<h2>1. Common Problems.</h2>

<a id="Q1.1"/>
<h3>1.1 How can I instruct glabels to not print blank address lines when doing a document merge?</h3>
<div class="faqanswer">
<p>
This feature can be best described by a simple example.  In the following CSV
file, column 5 contains the second address line for each record.  This field is
empty in records 1 and 2, but not in record 3.  (For this feature to work,
the field must be completely empty -- any text, including spaces will
defeat this feature.)
</p>
<div class="codebox">Summers,Joyce,,"1630 Revello Dr",,Sunnydale,CA,95037
McGarret,Steve,O,"404 Piikoi Street",,Honolulu,HI,96813
Kramer,Cosmo,,"Apt 5B","129 W. 81 St.","New York",NY,10024-7207
</div>
<p>
In the following screenshot, a single multiline text object has been
created to format these addresses.  Notice that ${5} representing
the second address line is on a line by itself.  (Any additional text
on this line, including spaces would defeat this feature.)
</p>
<?img("q1.3-screenshot.png","Multi-line address");?>
<p>
Printing this label results in the following output.
</p>
<?img("q1.3-output.png","Output");?>

</div>


<a id="S2"/>
<h2>2. Obsolete Problems (problems with older versions of gLabels).</h2>

<a id="Q2.1"/>
<h3>2.1 I cannot edit text.  What's wrong? (2.0.x)</h3>
<div class="faqanswer">
<p>
This is most likely due to an
<?a("http://bugzilla.gnome.org/show_bug.cgi?id=170031","outstanding bug");?> in
libbonoboui that was introduced in GNOME 2.8.  The symptom is that as soon
as you select a text object, its object property editor apears briefly then
disappears.
</p>
<p>
The workaround for this bug is to resize the glabels window so that it is large
enough to display the text property editor in the sidebar.
Note: it is impossible for glabels to automatically do this since the size
needed will vary with different themes, font sizes, and locales.
</p>
<p>
This problem is fixed in the development branch of glabels (2.1.x) by using
the GtkUIManager instead of libbonoboui to build the user interface.  These
changes are very extensive and require later versions of GNOME, thus are not
suitable for backport into the 2.0.x branch..
</p>
</div>


<a id="Q2.2"/>
<h3>2.2 Why don't transparent images print correctly? (1.92.1 - 2.1.3)</h3>
<div class="faqanswer">
<p>
There is currently a limitation in the postscript back-end of libgnomeprint
that causes images with an alpha channel to be composited in an intermediate
context at 72 DPI.  Unfortunately an image only needs to have an alpha channel
to trigger this behaviour even if the image is completely opaque, therefore
in earlier versions of gLabels (before 1.92.1) all GIF and PNG images would be
printed at 72 DPI.  To work around this problem image compositing was
sacrificed for full resolution rendering beginning with gLabels 1.92.1.
</p>
<p>
If you are compiling gLabels from source and are willing to sacrifice image
quality for image transparency, you can disable the workaround by
compiling glabels with the NO_ALPHA_HACK option:
</p>
<div class="codebox">$ ./configure CPPFLAGS=-DNO_ALPHA_HACK
$ make
...
</div>
</div>



<?php gLabelsFooter(); ?>
