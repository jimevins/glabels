<?php include "../common_php.inc"; ?>

<?php gLabelsHeader("news"); ?>

<h1 class="news">News ...</h1>




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




<div class="newsitem">
<div class="newsdate">21 February 2004</div>
<div class="newstitle">Release 1.93.3, "Doggy-Downers" (Unstable for GNOME 2)</div>
<p>
With any luck, this should be the final development release of gLabels before 2.0.  Changes/fixes include
</p>
<ul>

  <li>If GTK+ is version 2.3.2 or greater, the new GtkFileChooserDialog
  will be used for the <i>Open</i> and <i>Save As</i> dialogs.</li>

  <li>Added fixed format subtypes for POSTNET, EAN, UPC, and ISBN barcode
  types.</li>

  <li>Fixed resize problems with barcodes.</li>

  <li>Corner resize handles will now maintain the current constant aspect ratio
  of an object if the <i>Control</i> key is pressed while resizing.</li>

  <li>Fixed vertical offset of barcode text.</li>

  <li>Template code has been split out into its own library
  (<i>libglabels</i>) to facilitate 3rd party use of glabels templates.</li>

</ul>
</div>




<div class="newsitem">
<div class="newsdate">04 January 2004</div>
<div class="newstitle">Release 1.93.2, "Puppy-Uppers" (Unstable for GNOME 2)</div>
<p>
This latest development snapshot is primarily bug fixes, but includes
a couple of interesting new features.  Changes/fixes include
</p>
<ul>

  <li>Make templates known when encountered for the first time in a label. 
  This can occur when template names have changed between versions or
  when you receive a glabels document from someone else who has created a
  custom template. (me)</li>

  <li>Now builds against GNOME 2.5.1 (GARNOME 0.28.3). (me)</li>

  <li>Fixed button order in new label and label properties dialogs.
  (Wayne Schuller)</li>

  <li>Fixed a bug with the preferences dialog, which resulted in ignoring
  default object property preferences. (me)</li>

  <li>Fixed a bug which caused the waste property to function incorrectly
  on rectangular labels. (me)</li>

  <li>Added template designer assistant/wizard. (me)</li>

  <li>Added line spacing property to all text objects.  (Wayne Schuller)</li>

  <li>Fixed bug in object editor size page which caused the height spin
  button to be ignored. (me)</li>

  <li>Fixed bug with object editor size page that caused text object sizes
  to be prematurely changed when any property is edited. (me)</li>

</ul>
</div>




<div class="newsitem">
<div class="newsdate">23 December 2003</div>
<div class="newstitle">Release 1.93.1, "You'll shoot your eye out" (Unstable for GNOME 2)</div>
<p>
This latest development snapshot is primarily bug fixes.  Changes/fixes
include
</p>
<ul>
  <li>Document merge now honors locale's encoding when reading CSV files. (me)</li>
  <li>Size and position spinboxes now limit well beyond label edges. (me)</li>
  <li>Print dialog now tracks settings separately for each label/window. (me)</li>
  <li>Scroll window now tracks changes in template. (me)</li>
  <li>Various GUI elements are updated when preferences change. (me)</li>
  <li>Fixed position bug for box objects. (me)</li>
  <li>Fixed crash when copying text objects. (me)</li>
  <li>Added property bar entries to view menu. (me)</li>
  <li>Property bar widget now track properties of selected widgets in an intuitive manner (Emmanuel Pacaud)</li>
  <li>Fixed crash when a template name does not exactly match a current template name. (me)</li>
  <li>Some UI cleanup</li>
  <li>Fixed size reset button on image size pages. (me)</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">30 November 2003</div>
<div class="newstitle">Release 1.93.0, "More Goo To Go" (Unstable for GNOME 2)</div>
<p>
This latest development snapshot replaces the multiple object property dialogs with a single dockable sidebar.  This release completes the feature set planned for glabels 2.0.
</p>
</div>




<div class="newsitem">
<div class="newsdate">9 November 2003</div>
<div class="newstitle">Release 1.92.3, "Spam Lamb" (Unstable for GNOME 2)</div>
<p>
This latest development snapshot fixes a couple of nasty bugs:
</p>
<ul>
  <li>Fixes a crash that occured when changing zoom level after objects have been deleted.</li>
  <li>Templates with a page size of "Other" are once again supported.</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">1 November 2003</div>
<div class="newstitle">Release 1.92.2, "Book'im" (Unstable for GNOME 2)</div>
<p>
This latest development snapshot fixes the glabels-batch command line front-end, which has been broken since 1.92.0.
</p>
</div>




<div class="newsitem">
<div class="newsdate">27 October 2003</div>
<div class="newstitle">Release 1.92.1 "KMG365" (Unstable for GNOME 2)</div>
<p>
This latest development snapshot is primarily bug fixes.  Changes/fixes
include
</p>
<ul>
  <li>Added option to print crop marks. (Me)</li>
  <li>Fixed NULL name bug in gl_pixbuf_cache_remove_pixbuf(). (Me)</li>
  <li>Fixed bug when importing glabels-0.4 text items. (Me)</li>
  <li>Fixed locale problems when reading paper-sizes.xml. (Emmanuel Pacaud)</li>
  <li>Default font changed to "Sans" from "Helvetica." (Emmanuel Pacaud)</li>
  <li>Updated to most recent egg-recent-files.  (Emmanuel Pacaud)</li>
  <li>Fixed bug when text item is a single merge field.  (Emmanuel Pacaud)</li>
  <li>Fixed bug in hacktext canvas item that would leave artifacts when item is moved.   (Emmanuel Pacaud)</li>
  <li>Allow XML comments at any spot in a template file.  (Me)</li>
  <li>Workaround for poor rendering of images with alpha channel.  (Me)</li>
  <li>Properly clean-up bonobo ui component of destroyed windows.  (Me)</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">3 September 2003</div>
<div class="newstitle">Release 1.92.0 "Ain't No Big Thing, Brudda" (Unstable for GNOME 2)</div>

<p>
This latest development snapshot has been a long time in the making.  Changes
include
</p>
<ul>
  <li>Uses libgnomeprint-2.2</li>
  <li>Fixes a couple of crashes associated with image objects.</li>
  <li>A new property toolbar for changing common object properties "en masse."</li>
  <li>XML file formats have been cleaned up (see <?a("/doc/glabels-2.0.dtd.txt","glabels-2.0.dtd");?>).
    <ul>
      <li>Labels generated with 1.91.x should import automatically.</li>
    </ul>
  </li>
  <li>Templates now support a waste attribute, so that background images can print beyond the edge of the label.</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">26 January 2003</div>
<div class="newstitle">Release 1.91.1 (Unstable for GNOME 2)</div>

<p>This development snapshot includes</p>
<ul>
  <li>Support for "business card" CDs.</li>
  <li>Images are now saved "inline."</li>
  <li>Text boxes can be resized.</li>
  <li>Barcodes are resizable both vertically and horizontally.</li>
  <li>Barcode checksums can be enabled/disabled by the user.</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">31 December 2002</div>
<div class="newstitle">Release 1.91.0 (Unstable for GNOME 2)</div>

<p>This development snapshot includes</p>
<ul>
  <li>Object flip and rotation commands.</li>
  <li>Object alignment commands.</li>
  <li>A status bar, showing current zoom level and cursor position.</li>
  <li>Image filenames can come from a document merge field.</li>
  <li>A cleaner API for document merge backends.</li>
  <li>Ability to select individual records when performing a document merge.</li>
  <li>Templates support layouts beyond simple grids (see Memorex CD Labels for an example).</li>
  <li>Templates support markup lines.</li>
</ul>
<p>NOTE:  This development snapshot breaks compatibility with labels created with previous development versions.</p>
</div>




<div class="newsitem">
<div class="newsdate">05 October 2002</div>
<div class="newstitle">Release 1.91.0 (Unstable for GNOME 2)</div>

<p>This development snapshot includes</p>
<ul>
  <li>This release now uses a single document interface (SDI) instead of MDI.</li>
  <li>Template coordinate system is now relative to upper left corner.</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">26 September 2002</div>
<div class="newstitle">Release 1.89.2 (Unstable for GNOME 2)</div>

<p>This development snapshot includes</p>
<ul>
  <li>fixes several problems with document-merge feature</li>
  <li>extensive user interface tweaks</li>
  <li>more code cleanup</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">10 September 2002</div>
<div class="newstitle">Release 1.89.1 (Unstable for GNOME 2)</div>

<p>This development snapshot includes</p>
<ul>
  <li>the ability to read label files created with 0.4.x</li>
  <li>various user interface tweaks</li>
  <li>some code cleanup</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">10 September 2002</div>
<div class="newstitle">Release 0.4.6 (Stable for GNOME 1.4)</div>

<p>
Yet another release for the GNOME 1.4 platform.
</p>
<ul>
  <li>Fixes a bug which ignored isolated whitespace between merge fields when opening previously saved label files.</li>
  <li>Includes updated Russian language support, thanks to Vitaly Lipatov</li>
  <li>Additional label templates</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">27 August 2002</div>
<div class="newstitle">Release 1.89.0 (Unstable for GNOME 2)</div>

<p>
This is the first development release of the codebase for GNOME 2.  This
release is far from complete, and does still have problems.
<a href="/download/#unstable">Download</a> at your own risk.
</p>
</div>




<div class="newsitem">
<div class="newsdate">25 August 2002</div>
<div class="newstitle">Release 0.4.5</div>

<p>
"Ok, so I lied."  Hopefully, this will be the final release for GNOME 1.4.  Future releases will target the GNOME 2.0 platform.
</p>
<ul>
  <li>Fixes a locale bug, causing special characters to not be printed in some environments</li>
  <li>Fixes a bug in glabels-batch when it attempts to output over an existing file</li>
  <li>Includes Russian language support, thanks to Vitaly Lipatov</li>
  <li>Includes Chinese language support, thanks to Shell, Hin-lik Hung</li>
  <li>Includes Polish language support, thanks to Arkadiusz Lipiec</li>
  <li>Includes updated French language support, thanks to Olivier Berger</li>
  <li>Includes updated Japanese language support, thanks to Takeshi Aihana</li>
  <li>Additional label templates</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">27 June 2002</div>
<div class="newstitle">Release 0.4.4</div>

<p>
Hopefully, this will be the final release for GNOME 1.4.  Future releases will
target the GNOME 2.0 platform.
</p>
<ul>
  <li>This release now uses version 0.98 of <a href="http://arcana.linux.it/software/barcode/barcode.html">Alessandro Rubini's GNU-Barcode library</a></li>
  <li>The text back-end includes an improved CSV parser</li>
  <li>"Clip to outline" code has been removed -- it caused problems with some printers/drivers</li>
  <li>Fixes a bug in the canvas item event handlers</li>
  <li>Fixes origin bug in mini-preview widget</li>
  <li>Includes Spanish language support, thanks to Víctor Moral</li>
  <li>Additional label templates</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">22 Febuary 2002</div>
<div class="newstitle">Release 0.4.3</div>

<ul>
  <li>Includes a non-graphical front-end to print previously created labels.</li>
  <li>Added support for simple round labels</li>
  <li>Additional label templates.</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">03 Febuary 2002</div>
<div class="newstitle">Release 0.4.2</div>

<ul>
  <li>Updated French language support, thanks to Olivier Berger.</li>
  <li>Updated Japanese language support, thanks to Takeshi Aihana.</li>
  <li>Updated Brazilian Portuguese language support, thanks to Paulo Rogério Ormenese.</li>
  <li>Updated German language support, thanks to Marcus Bauer.</li>
  <li>Additional label templates.</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">14 January 2002</div>
<div class="newstitle">Release 0.4.1</div>

<ul>
  <li>Adds reverse (mirror-image) printing feature.</li>
  <li>Some code clean-up.</li>
  <li>Updated Japanese language support, thanks to Takeshi Aihana.</li>
  <li>Updated French language support, thanks to Olivier Berger.</li>
  <li>Additional label templates.</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">06 January 2002</div>
<div class="newstitle">Release 0.4.0</div>

<p>
This is the first release with a document merge feature.  The
implementation of this feature may still be a little rough around
the edges.  Currently only a simple back-end for text files exists --
others are planned.  The currently supported text-file format is very
simple:  each line is a record; fields are delimited by tabs,
commas, or colons; and newlines can be embedded into records by using
the "\n" entity.
</p>
<p>
Other changes include
</p>
<ul>
  <li>Some code clean-up.</li>
  <li>Updated Japanese language support, thanks to Takeshi Aihana. (0.3.4 functionality)</li>
  <li>Additional label templates.</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">17 December 2001</div>
<div class="newstitle">Release 0.3.4</div>

<ul>
  <li>Fixes focus behavior when using arrow keys to nudge objects.</li>
  <li>File and print dialogs now have memory across instances.</li>
  <li>This release includes Brazilian Portuguese language support, thanks to Paulo Rogério Ormenese.</li>
  <li>This release includes updated French language support, thanks to Olivier Berger.</li>
  <li>Additional label templates.</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">13 December 2001</div>
<div class="newstitle">Release 0.3.3</div>

<ul>
  <li>Fixes a crash when opening a label that has no corresponding template.</li>
  <li>This release no longer uses a single flat file for templates.  Separate user created template files can be placed in a ~/.glabels/ directory.</li>
  <li>The "New" dialog now segregates labels by paper size (i.e. US-Letter and A4).  The user can now select the default paper size in the "Preferences" dialog.</li>
  <li>The user can now select a default unit for distance (points, inches, or mm) in the "Preferences" dialog.</li>
  <li>Numerous tweaks to the various "Object properties" dialogs.</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">06 December 2001</div>
<div class="newstitle">Release 0.3.2</div>

<ul>
  <li>Fixes a bug in parsing of hexadecimal colors from label files.</li>
  <li>Adds nudge feature.  Selections can now be "nudged" using arrow keys.</li>
  <li>An additional label template.</li>
</ul>

<h2>03 December 2001, Release 0.3.1:</h2>
<ul>
  <li>Fixes segmentation fault with consecutive newlines in text objects.</li>
  <li>This release includes Japanese language support, thanks to Takeshi Aihana.</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">02 December 2001</div>
<div class="newstitle">Release 0.3.0</div>

<p>
This release includes some significant restructuring "under the hood."
Some of the more visible features include:
</p>
<ul>
  <li>Mini-previews have been added to the "New" dialog.</li>
  <li>Objects can now be resized by dragging on "handles" with the mouse.</li>
  <li>Object property dialogs include many improvements.</li>
  <li>Additional label templates.</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">18 November 2001</div>
<div class="newstitle">Release 0.2.4</div>

<ul>
  <li>This release adds a simple interface to the print dialog that allows the selection of individual or multiple labels to be printed.</li>
  <li>Additional label templates.</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">11 November 2001</div>
<div class="newstitle">Release 0.2.3</div>

<ul>
  <li>Zoom capability has been added.</li>
  <li>Additional label templates.</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">28 October 2001</div>
<div class="newstitle">Release 0.2.2</div>

<ul>
  <li>Object selection and cursor behavior have been slightly modified.  Objects can now be selected and moved with the same initial mouse click.</li>
  <li>This release introduces a <i>type</i> property to the label template, in preparation for non-rectangular label types (e.g. CD labels).</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">21 October 2001</div>
<div class="newstitle">Release 0.2.1</div>

<ul>
  <li>This release fixes a large portability problem with the anti-aliased text feature introduced in 0.2.0.</li>
  <li>This release includes German language support, thanks to Marcus Bauer.</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">14 October 2001</div>
<div class="newstitle">Release 0.2.0</div>

<ul>
  <li>This release supports rotated labels and cards.  I.e. cards and labels that would normally default to a landscape orientation can now be rotated to a portrait orientation. (There are some problems with image objects in this rotated
orientation during print-preview.)</li>
  <li>Text objects are now anti-aliased for a more WYSIWYG experience.
    <ul>
      <li>If you experience problems with text objects, this feature can be disabled by doing a "<tt>./configure --disable-hacktext</tt>" and recompiling.</li>
    </ul>
  </li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">05 October 2001</div>
<div class="newstitle">Release 0.1.5</div>

<p>
This quick release includes French language support, thanks to
Olivier Berger.
</p>
</div>




<div class="newsitem">
<div class="newsdate">01 October 2001</div>
<div class="newstitle">Release 0.1.4</div>

<p>
This release adds the standard clipboard functions (cut, 
copy and paste).  It also fixes a couple of minor bugs.
</p>
</div>




<div class="newsitem">
<div class="newsdate">26 September 2001</div>
<div class="newstitle">Release 0.1.3</div>

This release adds 10 additional barcode types, thanks to <a href="http://arcana.linux.it/software/barcode/barcode.html">Alessandro Rubini's GNU-Barcode library</a>.
</div>




<div class="newsitem">
<div class="newsdate">08 September 2001</div>
<div class="newstitle">Release 0.1.2</div>

<ul>
  <li>Improved handling of locale character sets.</li>
  <li>Aggregate item selection is now supported (multiple items can be manipulated as a group).</li>
  <li>Additional label templates.</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">03 September 2001</div>
<div class="newstitle">Release 0.1.1</div>

<ul>
  <li>Initial barcode support has been added (POSTNET and Code 39).  These have not been tested -- volunteers are welcome.</li>
  <li>An option to print label outlines has been added to test printer alignment.</li>
  <li>Additional label templates have been added.</li>
</ul>
</div>




<div class="newsitem">
<div class="newsdate">28 August 2001</div>
<div class="newstitle">Release 0.1.0</div>

<p>
This is the initial release of gLabels.  It is still a little
rough around the edges and is missing several important features;
however, it should have enough functionality to produce sheets
of identical labels or business cards.
</p>
</div>





<?php gLabelsFooter(); ?>
