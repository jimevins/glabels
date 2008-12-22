<?php include "common_php.inc"; ?>

<?php gLabelsHeader("home"); ?>

<h1 class="about">About gLabels ...</h1>

<p>
<b>gLabels</b> is a program for creating labels and business cards for the <i>GNOME</i> desktop
environment. It is designed to work with various laser/ink-jet peel-off label and business card sheets that
you'll find at most office supply stores. gLabels is free software and is distributed under the terms of the
<i>GNU General Public License</i> (<?a("http://www.gnu.org/copyleft/gpl.html","GPL");?>).
</p>




<h1 class="news">Recent News ...</h1>




<div class="newsitem">
<div class="newsdate">22 December 2008</div>
<div class="newstitle">Release 2.2.4 (Stable)</div>
<p>
Changes/fixes since 2.2.3 include
</p>
<ul>
<li>Corrected button order in "Open" and "Save as" dialogs.</li>
<li>Fixed performance problem when large number of fonts are installed.</li>
<li>Corrected several i18n problems.</li>
<li>Fixed "paste" bug that created phantom object views.</li>
<li>Fixed performacne problem when many objects are selected.</li>
<li>New templates.</li>
</ul>
<p>
<?a("/download/","Download here");?>.
</p>
</div>






<div class="newsitem">
<div class="newsdate">19 August 2008</div>
<div class="newstitle">Release 2.2.3 (Stable)</div>
<p>
Changes/fixes since 2.2.2 include
</p>
<ul>
<li>Explicitely set page size when printing -- don't let print back-end use
  a default size (Bug #2049431).</l>
<li>Sets more appropriate object size limits for any aspect ratio (Bug #1993550).</li>
<li>Fixed crash of glabels-batch (Bug #1968653).</li>
<li>Fixed bug with barcode and image merge radio buttons (Bug #1959135).</li>
<li>Will now attempt to parse newer namespaces (Bug #1872584).</li>
<li>Fixed drawing errors when drawing markup on rotated labels.</li>
<li>New korean translation (xein@navir.com).</li>
<li>New templates.</li>
</ul>
</div>






<div class="newsitem">
<div class="newsdate">22 February 2008</div>
<div class="newstitle">Release 2.2.2, "Walt Whitman High" (Stable)</div>
<p>
Changes/fixes since 2.2.1 include
</p>
<ul>
<li>Fixed crash when creating Code128C barcode (Bug #1766425).</li>
<li>Explicitely set page size when printing "other" page size.</li>
<li>Fixed rendering bug with bitmap fonts.</li>
<li>Fixed rendering problem when displaying "Invalid barcode data".</li>
<li>Fixed crash when deleting multi-object selections.</li>
<li>Enforce minimum size for images to prevent rendering of images of zero size.</li>
<li>Fixed problem with image size spinboxes (Bug #1886438).</li>
<li>Fixed template example in documentation.</li>
<li>Use default units when writing xml files.</li>
</ul>
</div>






<div class="newsitem">
<div class="newsdate">23 January 2008</div>
<div class="newstitle">Release 2.2.1 (Stable)</div>
<p>
Changes/fixes since 2.2.0 include
</p>
<ul>
<li>Font lists are now sorted alphabetically.</li>
<li>Evolution records are now sorted by the "file_under" field.</li>
<li>Fixed clipping bug when rotating labels.</li>
</ul>
</div>






<div class="newsitem">
<div class="newsdate">26 December 2007</div>
<div class="newstitle">Release 2.2.0 (Stable)</div>
<p>
<i>After two years in the making, it's finally here...</i>
</p>
<p>
<i>A new stable release</i>.  The underlying technology has been largely overhauled from the 2.0.x branch.
The libgnomeprint library has been completely replaced with Cairo and the new GtkPrintOperation API.
One side effect of print rendering with Cairo is that image compositing now works at full resolution.
Cairo has also replaced GnomeCanvas in the main display and drawing interface.
Many other deprecated dependencies have been replaced as well.  Because of this, this version requires at least Gnome 2.16.
</p>
<p>
Changes/fixes since 2.0.4 include
</p>
<ul>

<li>Migrated from EggRecent to GtkRecentManager</li>
<li>Migrated from libgnomeprint to GtkPrintOperation and Cairo.</li>
<li>Migrated from libgnomeprint to Pango for text layout.</li>
<li>Migrated GnomeDruid to GtkAssistant for template designer.</li>
<li>Migrated from GnomeCanvas to Cairo.</li>
<li>Removed all deprecated widgets as of Gnome-2.16/Gtk-2.10.</li>
<li>Added categorization of templates and an updated New Label dialog.</li>
<li>New label dialog now allows selection of recently used templates</li>
<li>Removed splash screen.</li>
<li>Added initial support for datamatrix 2D barcodes.</li>
<li>Added support for Brazilian CEPNet barcodes.</li>
<li>Added shadow attribute to label objects.</li>
<li>New merge backend for evolution data server</li>
<li>Various bug fixes.</li>
<li>New templates.</li>

</ul>
</div>






<div class="newsitem">
<div class="newsdate">9 December 2007</div>
<div class="newstitle">Release 2.1.5 (Unstable)</div>
<p>
This is hopefully the final development release before 2.2.  Please download
and test.
</p>
<p>
Changes/fixes since 2.1.4 include
</p>
<ul>
<li>New label dialog now allows selection of recently used templates</li>
<li>Store brand and part number as separate fields in templates</li>
<li>Major cleanup of libglabels API and documentation</li>
<li>Minor bug fixes (and probably some new bugs).</li>
</ul>
</div>






<div class="newsitem">
<div class="newsdate">17 August 2007</div>
<div class="newstitle">Release 2.1.4 (Unstable)</div>
<p>
<i>It's been over a year in the making...</i>
</p>
<p>
Although this latest development snapshot has only a few functional updates, its underlying technology has been largely overhauled.
The libgnomeprint library has been completely replaced with the new GtkPrintOperation and Cairo.
One side effect of print rendering with Cairo is that image compositing now works at full resolution.
Cairo has also replaced GnomeCanvas in the main display and drawing interface.
Many other deprecated dependencies have been replaced as well.  Because of this, this version requires at least Gnome 2.16.
</p>
<p>
Changes/fixes since 2.1.3 include
</p>
<ul>

<li>Added categorization of templates and an updated New Label dialog.</li>
<li>Added initial support for datamatrix 2D barcodes.</li>
<li>Migrated from EggRecent to GtkRecentManager.</li>
<li>Migrated from libgnomeprint to GtkPrintOperation and Cairo.</li>
<li>Migrated from libgnomeprint to Pango for text layout.</li>
<li>Migrated GnomeDruid to GtkAssistant for template designer.</li>
<li>Migrated from GnomeCanvas to Cairo.</li>
<li>Removed splash screen.</li>
<li>Added support for Brazilian CEPNet barcodes.</li>
<li>Updated French translation (Claude Paroz)</li>
<li>New Esperanto translation (Antonio C. Codazzi)</li>
<li>Minor bug fixes (and probably some new bugs).</li>
<li>New templates.</li>

</ul>
</div>






<div class="newsitem">
<div class="newsdate">27 May 2006</div>
<div class="newstitle">Release 2.1.3 (Unstable)</div>
<p>
Changes/fixes since 2.1.2 include
</p>
<ul>

<li>Don't print "Invalid barcode data" if data field is empty. Fixes Bug #1479839. (Darren Warner)</li>
<li>Fixed hangup when grid or markup have been toggled during a previous session. Fixes Bug #1434324.</li>
<li>Added shadow attribute to label objects.</li>
<li>Fixed color groups in color combo widgets.</li>
<li>Fixed several potential crashes.</li>

</ul>
</div>






<div class="newsitem">
<div class="newsdate">26 December 2005</div>
<div class="newstitle">Release 2.0.4 (Stable)</div>
<p>
Changes/fixes since 2.0.3 include
</p>
<ul>

<li>Use filechooser for GnomeFileEntrys in merge properties dialog and
  object editor image page.  Fixes #1328173 and #1294692</li>
<li>New configure option to disable update-mime-database and update-desktop-database</li>
<li>Minor cleanup</li>
<li>New templates and fixes</li>

</ul>
</div>






<div class="newsitem">
<div class="newsdate">22 December 2005</div>
<div class="newstitle">Release 2.1.2 (Unstable)</div>
<p>
Changes/fixes since 2.1.1 include
</p>
<ul>

<li>Updated Italian translation (Daniele Medri)</li>
<li>Fixed double entry in property bar's font family combobox</li>
<li>Don't allow zero width or height when drawing pixbufs</li>
<li>Fixed various memory leaks</li>
<li>New configure option to disable update-mime-database and update-desktop-database</li>
<li>New merge backend for evolution data server (Austin Henry)</li>
<li>New merge backend for VCards (Austin Henry)</li>
<li>New templates and fixes</li>

</ul>
</div>






<div class="newsitem">
<div class="newsdate">26 June 2005</div>
<div class="newstitle">Release 2.1.1 (Unstable)</div>
<p>
Changes/fixes since 2.1.0 include
</p>
<ul>

<li>Added document merge tutorial</li>
<li>Replaced deprecated widgets (now requires Gnome 2.10 and GTK+ 2.6)</li>
<li>Replaced libbonoboui with GtkUIManager (now requires Gnome 2.10 and GTK+ 2.6)</li>
<li>Updated Russian translation (Vitaly Lipatov)</li>
<li>Updated Spanish translation (Sergio Rivadero)</li>
<li>Updated Czech translation (David Makovský and Zbynek Mrkvick)</li>
<li>Updated Italian translation (Daniele Medri)</li>
<li>Added documentation for libglabels</li>
<li>Minor bug fixes and cleanup</li>
<li>New templates</li>

</ul>
</div>






<div class="newsitem">
<div class="newsdate">26 June 2005</div>
<div class="newstitle">Release 2.0.3 (Stable)</div>
<p>
Changes/fixes since 2.0.2 include
</p>
<ul>

<li>Added document merge tutorial</li>
<li>Updated Russian translation (Vitaly Lipatov)</li>
<li>Updated Spanish translation (Sergio Rivadero)</li>
<li>Updated Czech translation (David Makovský and Zbynek Mrkvick)</li>
<li>Updated Italian translation (Daniele Medri)</li>
<li>Fixed packing style of printer selection dialog</li>
<li>New templates</li>

</ul>
</div>






<div class="newsitem">
<div class="newsdate">22 January 2005</div>
<div class="newstitle">Release 2.1.0 (Unstable)</div>
<p>
This is the first development release in preparation for 2.2.  New features include
</p>
<ul>

<li>Color object properties can be provided through document-merge feature (Frederic Ruaudel)</li>
<li>Fixes/changes from 2.0.2</li>

</ul>
</div>






<div class="newsitem">
<div class="newsdate">22 January 2005</div>
<div class="newstitle">Release 2.0.2 (Stable)</div>
<p>
Changes/fixes since 2.0.1 include
</p>
<ul>

<li>Desktop file reconciled with desktop-entry-spec</li>
<li>New cleaned-up icon</li>
<li>Added option to glabels-batch to allow selection of first label</li>
<li>Added options to glabels-batch to allow selection of merge source and crop marks (José Dapena Paz)</li>
<li>Updated french translation (Florent Morel)</li>
<li>Fixed translation problem with object editor labels</li>
<li>Minor bug fixes and cleanup</li>
<li>New templates</li>

</ul>
</div>






<div class="newsitem">
<div class="newsdate">15 August 2004</div>
<div class="newstitle">Release 2.0.1, "You know it, Bruddah" (Stable)</div>
<p>
There were a couple of problems with 2.0.0.  Changes/fixes include
</p>
<ul>

<li>Fixed crash when reading some labels created with 1.93.x</li>
<li>Updated japanese translations (Takeshi AIHANA)</li>
<li>Fixed crash from improperly freeing translation strings</li>

</ul>
</div>






<div class="newsitem">
<div class="newsdate">09 August 2004</div>
<div class="newstitle">Release 2.0.0, "Zulu as Kono" (Stable)</div>
<p>
Well it's finally here, 2.0 -- woo hoo!
Changes/fixes from 1.93.3 include
</p>
<ul>

<li>Fixed crash when encountering old template files</li>

<li>Align grid to label center for DVD/CD and round labels</li>

<li>Don't touch compression of original file</li>

<li>64-bit fixes</li>

<li>Added man page</li>

<li>Fixed missing outline problem experienced with some printers</li>

<li>Added separate horizontal and vertical waste properties for rectangular templates</li>

<li>Added option to auto-shrink text during document merge</li>

<li>Fixed memory leaks</li>

<li>Added format attibute to barcode objects, to allow correct layout of merged labels</li>

<li>Added mime type registration for GNOME 2.6</li>

</ul>
</div>







<h2><?a("/news/","Older news ...");?></h2>


<?php gLabelsFooter(); ?>
