<?php include "../../common_php.inc"; ?>

<?php gLabelsHeader("docs"); ?>

<h1 class="doc">Installing gLabels from Source</h1>

<p>
The following outlines how to install glabels from source on various GNU/Linux distributions.
</p>


<h2>Prerequisites</h2>

<p>
Dependencies will need to be installed as root.
Although dependecies can be installed using your distribution's graphical package manager, the directions below use command line tools because it is easier to convey.
</p>

<h4>Fedora and similar distributions</h4>
<div class="codebox">$ su
Password:
# yum groupinstall "Development Tools"
# yum groupinstall "GNOME Software Development"
</div>

<h4>Ubuntu and similar distributions</h4>
<div class="codebox">$ sudo apt-get install libgtk2.0-dev libgnomeui-dev libxml2-dev libglade2-dev
$ sudo apt-get install libebook1.2-dev <i>(optional)</i>
</div>

<p>
The exact dependencies and their names may vary with different distributions and versions.  There may also be additional dependencies depending on the baseline configuation of your distribution.  If <tt>./configure</tt> in the following step complains, you may need to return to this step and install additional dependencies.
</p>


<h2>Unpack and Build</h2>

<p>This step should not vary with distribution.</p>

<div class="codebox">$ tar xvfz glabels-2.2.x.tar.gz
$ cd glabels-2.2.x
$ ./configure
$ make
</div>


<h2>Install</h2>

<p>
Installation will need to be done as root.
</p>

<h4>Fedora and similar distributions</h4>
<div class="codebox">$ su
Password:
# make install
</div>

<h4>Ubuntu and similar distributions</h4>
<div class="codebox">$ sudo make install
</div>


<?php gLabelsFooter(); ?>
