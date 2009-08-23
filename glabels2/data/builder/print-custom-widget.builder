<?xml version="1.0"?>
<interface>
  <requires lib="gtk+" version="2.16"/>
  <!-- interface-naming-policy toplevel-contextual -->
  <object class="GtkWindow" id="window1">
    <property name="visible">True</property>
    <property name="title" translatable="yes">window1</property>
    <child>
      <object class="GtkVBox" id="print_custom_widget_vbox">
        <property name="visible">True</property>
        <property name="border_width">12</property>
        <property name="spacing">12</property>
        <child>
          <object class="GtkVBox" id="vbox3">
            <property name="visible">True</property>
            <child>
              <object class="GtkFrame" id="simple_frame">
                <property name="visible">True</property>
                <property name="label_xalign">0</property>
                <property name="shadow_type">none</property>
                <child>
                  <object class="GtkAlignment" id="alignment1">
                    <property name="visible">True</property>
                    <property name="left_padding">12</property>
                    <child>
                      <object class="GtkVBox" id="copies_vbox">
                        <property name="visible">True</property>
                        <child>
                          <placeholder/>
                        </child>
                      </object>
                    </child>
                  </object>
                </child>
                <child type="label">
                  <object class="GtkLabel" id="label2">
                    <property name="visible">True</property>
                    <property name="label" translatable="yes">&lt;b&gt;Print control (Simple)&lt;/b&gt;</property>
                    <property name="use_markup">True</property>
                  </object>
                </child>
              </object>
              <packing>
                <property name="position">0</property>
              </packing>
            </child>
            <child>
              <object class="GtkFrame" id="merge_frame">
                <property name="visible">True</property>
                <property name="label_xalign">0</property>
                <property name="shadow_type">none</property>
                <child>
                  <object class="GtkAlignment" id="alignment2">
                    <property name="visible">True</property>
                    <property name="left_padding">12</property>
                    <child>
                      <object class="GtkVBox" id="prmerge_vbox">
                        <property name="visible">True</property>
                        <child>
                          <placeholder/>
                        </child>
                      </object>
                    </child>
                  </object>
                </child>
                <child type="label">
                  <object class="GtkLabel" id="label3">
                    <property name="visible">True</property>
                    <property name="label" translatable="yes">&lt;b&gt;Print control (Merge)&lt;/b&gt;</property>
                    <property name="use_markup">True</property>
                  </object>
                </child>
              </object>
              <packing>
                <property name="position">1</property>
              </packing>
            </child>
          </object>
          <packing>
            <property name="expand">False</property>
            <property name="fill">False</property>
            <property name="position">0</property>
          </packing>
        </child>
        <child>
          <object class="GtkFrame" id="frame3">
            <property name="visible">True</property>
            <property name="label_xalign">0</property>
            <property name="shadow_type">none</property>
            <child>
              <object class="GtkAlignment" id="alignment3">
                <property name="visible">True</property>
                <property name="left_padding">12</property>
                <child>
                  <object class="GtkVBox" id="vbox2">
                    <property name="visible">True</property>
                    <child>
                      <object class="GtkCheckButton" id="outline_check">
                        <property name="label" translatable="yes">print outlines (to test printer alignment)</property>
                        <property name="visible">True</property>
                        <property name="can_focus">True</property>
                        <property name="receives_default">False</property>
                        <property name="use_underline">True</property>
                        <property name="draw_indicator">True</property>
                      </object>
                      <packing>
                        <property name="expand">False</property>
                        <property name="fill">False</property>
                        <property name="position">0</property>
                      </packing>
                    </child>
                    <child>
                      <object class="GtkCheckButton" id="reverse_check">
                        <property name="label" translatable="yes">print in reverse (i.e. a mirror image)</property>
                        <property name="visible">True</property>
                        <property name="can_focus">True</property>
                        <property name="receives_default">False</property>
                        <property name="use_underline">True</property>
                        <property name="draw_indicator">True</property>
                      </object>
                      <packing>
                        <property name="expand">False</property>
                        <property name="fill">False</property>
                        <property name="position">1</property>
                      </packing>
                    </child>
                    <child>
                      <object class="GtkCheckButton" id="crop_marks_check">
                        <property name="label" translatable="yes">print crop marks</property>
                        <property name="visible">True</property>
                        <property name="can_focus">True</property>
                        <property name="receives_default">False</property>
                        <property name="use_underline">True</property>
                        <property name="draw_indicator">True</property>
                      </object>
                      <packing>
                        <property name="expand">False</property>
                        <property name="fill">False</property>
                        <property name="position">2</property>
                      </packing>
                    </child>
                  </object>
                </child>
              </object>
            </child>
            <child type="label">
              <object class="GtkLabel" id="label4">
                <property name="visible">True</property>
                <property name="label" translatable="yes">&lt;b&gt;Options&lt;/b&gt;</property>
                <property name="use_markup">True</property>
              </object>
            </child>
          </object>
          <packing>
            <property name="position">1</property>
          </packing>
        </child>
      </object>
    </child>
  </object>
</interface>
