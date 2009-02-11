<?php include "../common_php.inc"; ?>

<?php gLabelsHeader("contact"); ?>

<h1 class="contact">Contact ...</h1>

<h2>Reporting bugs</h2>
<p>Please report any bugs you find in gLabels at the 
<?a("http://sourceforge.net/tracker/?atid=445116&amp;group_id=46122&amp;func=browse","SourceForge gLabels Bug Tracker");?>.
You may also join the <?a("http://lists.sourceforge.net/lists/listinfo/glabels-devel","gLabels Developer List");?> to discuss
the problem more directly (please submit all bugs to the tracker as well).
</p>
<p>Please consult the following checklist when reporting a bug.
</p>
<ol>
  <li>Verify that you are not experiencing a known bug or limitation by consulting the <?a("/faq/", "gLabels FAQ");?>.</li>
  <li>Verify that the bug is not a duplicate of an existing bug by searching the tracker.  If you have additional information
      on an existing bug, please add your information to the existing bug.  If you
      are not using the latest version of glabels, you should also look for closed
      bugs since the problem may have already been resolved.</li>
  <li>Please provide complete information:
      <ul>
        <li>version of gLabels</li>
        <li>platform (operating system, version, and CPU)</li>
        <li>detailed steps to reproduce the problem</li>
        <li>sample file(s) (if appropriate)</li>
        <li>screenshots (if appropriate)</li>
        <li>locale (very important for i18n issues, e.g. problems with special characters)</li>
      </ul>
  </li>
  <li>If this is a crash, it is important to submit a backtrace. Use gdb to get a backtrace as follows:
<div class="codebox">$ <b>gdb glabels</b>
GNU gdb ...

(gdb) <b>run</b>
        <b><i>(Carry out the actions to make glabels crash)</i></b>
(gdb) <b>bt</b>
        <b><i>(copy and paste all of this to your bug report)</i></b>
(gdb) <b>quit</b>
$</div>
  </li>

</ol>

<h2>Feature Requests</h2>
Feature requests should be submitted to the
<?a("http://sourceforge.net/tracker/?atid=445119&amp;group_id=46122&amp;func=browse","Sourceforge gLabels Feature Request Tracker");?>.

<h2>Mailing Lists</h2>
<ul>
  <li>Developers list: <?a_mailto("glabels-devel@lists.sourceforge.net");?>
    <ul>
      <li><?a("http://lists.sourceforge.net/lists/listinfo/glabels-devel","Join");?></li>
      <li><?a("http://sourceforge.net/mailarchive/forum.php?forum=glabels-devel","Archives");?></li>
    </ul>
  </li>
</ul>

<ul>
  <li>Template designers list: <?a_mailto("glabels-templates@lists.sourceforge.net");?>
    <ul>
      <li><?a("http://lists.sourceforge.net/lists/listinfo/glabels-templates","Join");?></li>
      <li><?a("http://sourceforge.net/mailarchive/forum.php?forum=glabels-templates","Archives");?></li>
    </ul>
  </li>
</ul>
  

<h2>Want to help?</h2>
<p>
Please start by joining one or both of the above mailing lists.
</p>




<?php gLabelsFooter(); ?>
