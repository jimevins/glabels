<?xml version="1.0"?>
<interface>
  <requires lib="gtk+" version="2.16"/>
  <!-- interface-naming-policy toplevel-contextual -->
  <object class="GtkWindow" id="window1">
    <property name="visible">True</property>
    <property name="title" translatable="yes">window1</property>
    <child>
      <object class="GtkVBox" id="new_label_dialog_vbox">
        <property name="visible">True</property>
        <property name="border_width">12</property>
        <property name="spacing">12</property>
        <child>
          <object class="GtkFrame" id="frame1">
            <property name="visible">True</property>
            <property name="label_xalign">0</property>
            <property name="shadow_type">none</property>
            <child>
              <object class="GtkAlignment" id="alignment1">
                <property name="visible">True</property>
                <property name="left_padding">12</property>
                <child>
                  <object class="GtkVBox" id="media_select_vbox">
                    <property name="visible">True</property>
                    <child>
                      <placeholder/>
                    </child>
                  </object>
                </child>
              </object>
            </child>
            <child type="label">
              <object class="GtkLabel" id="label1">
                <property name="visible">True</property>
                <property name="label" translatable="yes">&lt;b&gt;Media type&lt;/b&gt;</property>
                <property name="use_markup">True</property>
              </object>
            </child>
          </object>
          <packing>
            <property name="position">0</property>
          </packing>
        </child>
        <child>
          <object class="GtkFrame" id="frame2">
            <property name="visible">True</property>
            <property name="label_xalign">0</property>
            <property name="shadow_type">none</property>
            <child>
              <object class="GtkAlignment" id="alignment2">
                <property name="visible">True</property>
                <property name="left_padding">12</property>
                <child>
                  <object class="GtkVBox" id="rotate_label_vbox">
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
                <property name="label" translatable="yes">&lt;b&gt;Label orientation&lt;/b&gt;</property>
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
