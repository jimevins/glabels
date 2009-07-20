<?php include "../common_php.inc"; ?>

<?php gLabelsHeader("docs"); ?>

<h1 class="doc">Documentation ...</h1>




<h2>Installing gLabels from source</h2>
<p>
The easiest way to install glabels is to use you distribution's package repository.  However, if the version
offered by your distribution is not the latest stable version, you may wish to install it manually from
source.
</p>
<ul>
  <li><?a("install-src/","Installing gLabels from Source");?></li>
</ul>



<h2>Customizing</h2>
<p>
gLabels knows about a small universe of label/card stationery. You may create new templates by using the <i>Template Designer</i> under the <i>File</i> menu.
</p>
<p>
If for some reason you wish to manually create templates, please read the appropriate page below. New templates can be mailed to 
<?a_mailto("glabels-templates@lists.sourceforge.net");?> 
for possible inclusion in future releases of gLabels. 
</p>
<ul>
  <li><?a("templates-2.2/","Designing Custom Templates for gLabels 2.2.x");?></li>
</ul>




<?php gLabelsFooter(); ?>
