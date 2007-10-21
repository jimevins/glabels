<?php include "../../common_php.inc"; ?>

<?php gLabelsHeader("docs"); ?>

<h1 class="doc">Defining new gLabels templates for gLabels 2.0.x</h1>

<p>
Predefined templates are defined by XML files located in
<i>${prefix}/share/glabels/</i>,
where <i>${prefix}</i> is usually something like
<i>/usr/local</i> or <i>/usr</i> depending
on the configuration option <i>prefix</i>.  gLabels will
use all files of the form <i>*-templates.xml</i> or <i>*.template</i>,
that it finds in <i>${prefix}/share/glabels/</i> and <i>${HOME}/.glabels/</i>.
Additional templates can be added by creating additional
<i>*.template</i> files in either of these directories.
</p>
<ul>
  <li>The format for these files is defined in the DTD: <?a("/doc/glabels-2.0.dtd.txt","glabels-2.0.dtd");?>. (This DTD also describes other XML formats used by glabels.)</li>
</ul>

<h2>Assumptions/caveats</h2>
<ul>
  <li>A sheet contains only one size of label or card (if a sheet contains more than one size of item, it can be split into multiple templates for multiple pass printing)</li>
  <li>Distances can be expressed in units of <i>pt</i>, <i>in</i>, <i>mm</i>, <i>cm</i>, or <i>pc</i>.  For example: "1.0in" or "2.54cm".  If no units are specified, computer points (<i>pt</i>) will be assumed (1 pt = 1/72 in = 0.352778 mm).</li>
</ul>


<h2><i>*-templates.xml</i> and <i>*.template</i> files</h2>
<div class="codebox">
&lt;?xml version="1.0"?&gt;
&lt;Glabels-templates&gt;

   ...<i>templates</i>...

&lt;/Glabels-templates&gt;
</div>


<h2>Example Template</h2>
<div class="codebox">
  &lt;Template name="Avery 8160" size="US-Letter" description="Mailing Labels"&gt;
    &lt;Label-rectangle id="0" width="189" height="72" round="5"&gt;
      &lt;Markup-margin size="5"/&gt;
      &lt;Layout nx="3" ny="10" x0="11.25" y0="36" dx="200" dy="72"/&gt;
    &lt;/Label-rectangle&gt;
    &lt;Alias name="Avery 5160"/&gt;
    &lt;Alias name="Avery 6233"/&gt;
  &lt;/Template&gt;
</div>


<h2>Template Node</h2>
<p>
A <i>Template</i> node describes a single stationary product.  It must contain
one instance of any type of Label node (<i>Label-rectangle</i>,
<i>Label-round</i>, or <i>Label-cd</i>).  This node can be followed by zero
or more <i>Alias</i> nodes.
</p>
<h3>Properties</h3>
<table border="1">
  <tr>
    <td valign="top">name</td>
    <td>Name or part # of stationary product.  E.g. "Avery 8160"</td>
  </tr>
  <tr>
    <td valign="top">size</td>
    <td>Size of sheet.  E.g., "US-Letter," "A4", ...</td>
  </tr>
  <tr>
    <td valign="top">description</td>
    <td>Description of stationary product.  E.g, "Mailing Labels."</td>
  </tr>
  <tr>
    <td valign="top">_description</td>
    <td>Translatable description of stationary product.  E.g, "Mailing Labels." (Only useful for predefined templates)</td>
  </tr>
  <tr>
    <td valign="top">width</td>
    <td>Page width.  Only valid if size="Other"</td>
  </tr>
  <tr>
    <td valign="top">height</td>
    <td>Page height.  Only valid if size="Other"</td>
  </tr>
</table>

<h2>Label-rectangle Node</h2>
<p>
A <i>Label-rectangle</i> node describes the dimensions of a single label or
business card that is rectangular in shape (may have rounded edges).
</p>
<h3>Properties</h3>
<table border="1">
  <tr>
    <td valign="top">id</td>
    <td>Reserved for future use.  Should always be 0.</td>
  </tr>
  <tr>
    <td valign="top">width</td>
    <td>Width of label/card</td>
  </tr>
  <tr>
    <td valign="top">height</td>
    <td>Height of label/card</td>
  </tr>
  <tr>
    <td valign="top">round</td>
    <td>Radius of corners.  For items with square edges (business cards)
        , the radius should be 0.</td>
  </tr>
  <tr>
    <td valign="top">x_waste</td>
    <td>Amount of horizontal waste (over-print) to allow.  This is useful for minimizing alignment problems when using non-white backgrounds (e.g. images).</td>
  </tr>
  <tr>
    <td valign="top">y_waste</td>
    <td>Amount of vertical waste (over-print) to allow.  This is useful for minimizing alignment problems when using non-white backgrounds (e.g. images).</td>
  </tr>
</table>
<br />
<?img("glabels-template-label.png","Label illustration");?>

<h2>Label-round Node</h2>
<p>
A <i>Label-round</i> node describes the dimensions of a simple round label (not a CD).
</p>
<h3>Properties</h3>
<table border="1">
  <tr>
    <td valign="top">id</td>
    <td>Reserved for future use.  Should always be 0.</td>
  </tr>
  <tr>
    <td valign="top">radius</td>
    <td>Radius (1/2 diameter) of label</td>
  </tr>
  <tr>
    <td valign="top">waste</td>
    <td>Amount of waste (over-print) to allow.  This is useful for minimizing alignment problems when using non-white backgrounds (e.g. images).</td>
  </tr>
</table>

<h2>Label-cd Node</h2>
<p>
A <i>Label-cd</i> node describes the dimensions of a CD, DVD, or business card
CD.
</p>
<h3>Properties</h3>
<table border="1">
  <tr>
    <td valign="top">id</td>
    <td>Reserved for future use.  Should always be 0.</td>
  </tr>
  <tr>
    <td valign="top">radius</td>
    <td>Outer radius of label</td>
  </tr>
  <tr>
    <td valign="top">hole</td>
    <td>Radius of concentric hole</td>
  </tr>
  <tr>
    <td valign="top">width</td>
    <td>If present, the label is clipped to the given width.  (Useful for "business card CDs").</td>
  </tr>
  <tr>
    <td valign="top">height</td>
    <td>If present, the label is clipped to the given height.  (Useful for "business card CDs").</td>
  </tr>
  <tr>
    <td valign="top">waste</td>
    <td>Amount of waste (over-print) to allow.  This is useful for minimizing alignment problems when using non-white backgrounds (e.g. images).</td>
  </tr>
</table>
<br />
<?img("glabels-template-cd.png","CD Label illustration");?>


<h2>Markup-margin Node</h2>
<p>
A <i>Markup-margin</i> describes a margin along all edges of a label.
</p>
<h3>Properties</h3>
<table border="1">
  <tr>
    <td valign="top">size</td>
    <td>Size of the margin.  I.e. the distance of the margin line from the
        edge of the card/label.</td>
  </tr>
</table>


<h2>Markup-line Node</h2>
<p>
A <i>Markup-line</i> describes a markup line.
</p>
<h3>Properties</h3>
<table border="1">
  <tr>
    <td valign="top">x1</td>
    <td>x coordinate of 1st endpoint of the line segment.</td>
  </tr>
  <tr>
    <td valign="top">y1</td>
    <td>y coordinate of 1st endpoint of the line segment.</td>
  </tr>
  <tr>
    <td valign="top">x2</td>
    <td>x coordinate of 2nd endpoint of the line segment.</td>
  </tr>
  <tr>
    <td valign="top">y2</td>
    <td>y coordinate of 2nd endpoint of the line segment.</td>
  </tr>
</table>

<h2>Markup-circle Node</h2>
<p>
A <i>Markup-circle</i> describes a markup circle.
</p>
<h3>Properties</h3>
<table border="1">
  <tr>
    <td valign="top">x0</td>
    <td>x coordinate of circle origin (center).</td>
  </tr>
  <tr>
    <td valign="top">y0</td>
    <td>y coordinate of circle origin (center).</td>
  </tr>
  <tr>
    <td valign="top">radius</td>
    <td>Radius of circle.</td>
  </tr>
</table>

<h2>Layout Node</h2>
<p>
A label node may contain multiple <i>Layout</i> children.  If labels are layed out in
a simple grid pattern, only one layout is needed.  However, if labels are
layed out in multiple grids, such as a running bond pattern, multiple
<i>layout</i> tags can be used.  Note: a single label can always be treated as
a grid of one.
</p>
<h3>Properties</h3>
<table border="1">
  <tr>
    <td valign="top">nx</td>
    <td>Number of labels/cards across in the grid (horizontal)</td>
  </tr>
  <tr>
    <td valign="top">ny</td>
    <td>Number of labels/cards up and down in the grid (vertical)</td>
  </tr>
  <tr>
    <td valign="top">x0</td>
    <td>Distance from left edge of sheet to the left edge of the left
	column of cards/labels in the layout.</td>
  </tr>
  <tr>
    <td valign="top">y0</td>
    <td>Distance from the top edge of sheet to the top edge of
        the top row of labels/cards in the layout.</td>
  </tr>
  <tr>
    <td valign="top">dx</td>
    <td>Horizontal pitch of grid.</td>
  </tr>
  <tr>
    <td valign="top">dy</td>
    <td>Vertical pitch of grid.</td>
  </tr>
</table>
<br />
<?img("glabels-template-layout.png","Layout illustration");?>

<h2>Alias Node</h2>
<h3>Properties</h3>
<table border="1">
  <tr>
    <td valign="top">name</td>
    <td>The name or part # of a product with the same size and layout
        characteristics.</td>
  </tr>
</table>

<br />


<?php gLabelsFooter(); ?>
