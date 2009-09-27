<?xml version="1.0"?>
<interface>
  <requires lib="gtk+" version="2.16"/>
  <!-- interface-naming-policy toplevel-contextual -->
  <object class="GtkWindow" id="window1">
    <property name="visible">True</property>
    <property name="title" translatable="yes">window1</property>
    <child>
      <object class="GtkToolbar" id="property_toolbar">
        <property name="visible">True</property>
        <property name="toolbar_style">icons</property>
        <property name="show_arrow">False</property>
        <child>
          <object class="GtkToolItem" id="toolitem1">
            <property name="visible">True</property>
            <child>
              <object class="GtkEventBox" id="font_family_eventbox">
                <property name="visible">True</property>
                <property name="tooltip_markup">Font family</property>
                <property name="visible_window">False</property>
                <child>
                  <placeholder/>
                </child>
              </object>
            </child>
          </object>
          <packing>
            <property name="expand">False</property>
          </packing>
        </child>
        <child>
          <object class="GtkToolItem" id="toolitem2">
            <property name="visible">True</property>
            <child>
              <object class="GtkAlignment" id="alignment1">
                <property name="visible">True</property>
                <property name="left_padding">6</property>
                <child>
                  <object class="GtkSpinButton" id="font_size_spin">
                    <property name="visible">True</property>
                    <property name="can_focus">True</property>
                    <property name="tooltip_text" translatable="yes">Font size</property>
                    <property name="adjustment">adjustment1</property>
                    <property name="climb_rate">1</property>
                  </object>
                </child>
              </object>
            </child>
          </object>
          <packing>
            <property name="expand">False</property>
          </packing>
        </child>
        <child>
          <object class="GtkSeparatorToolItem" id="separatortoolitem1">
            <property name="visible">True</property>
          </object>
          <packing>
            <property name="expand">False</property>
          </packing>
        </child>
        <child>
          <object class="GtkToggleToolButton" id="font_bold_toggle">
            <property name="visible">True</property>
            <property name="tooltip_text" translatable="yes">Bold</property>
            <property name="use_underline">True</property>
            <property name="stock_id">gtk-bold</property>
          </object>
          <packing>
            <property name="expand">False</property>
            <property name="homogeneous">True</property>
          </packing>
        </child>
        <child>
          <object class="GtkToggleToolButton" id="font_italic_toggle">
            <property name="visible">True</property>
            <property name="tooltip_text" translatable="yes">Italic</property>
            <property name="use_underline">True</property>
            <property name="stock_id">gtk-italic</property>
          </object>
          <packing>
            <property name="expand">False</property>
            <property name="homogeneous">True</property>
          </packing>
        </child>
        <child>
          <object class="GtkSeparatorToolItem" id="separatortoolitem2">
            <property name="visible">True</property>
          </object>
          <packing>
            <property name="expand">False</property>
          </packing>
        </child>
        <child>
          <object class="GtkRadioToolButton" id="text_align_left_radio">
            <property name="visible">True</property>
            <property name="tooltip_text" translatable="yes">Left align</property>
            <property name="use_underline">True</property>
            <property name="stock_id">gtk-justify-left</property>
          </object>
          <packing>
            <property name="expand">False</property>
            <property name="homogeneous">True</property>
          </packing>
        </child>
        <child>
          <object class="GtkRadioToolButton" id="text_align_center_radio">
            <property name="visible">True</property>
            <property name="tooltip_text" translatable="yes">Center align</property>
            <property name="use_underline">True</property>
            <property name="stock_id">gtk-justify-center</property>
            <property name="group">text_align_left_radio</property>
          </object>
          <packing>
            <property name="expand">False</property>
            <property name="homogeneous">True</property>
          </packing>
        </child>
        <child>
          <object class="GtkRadioToolButton" id="text_align_right_radio">
            <property name="visible">True</property>
            <property name="tooltip_text" translatable="yes">Right align</property>
            <property name="use_underline">True</property>
            <property name="stock_id">gtk-justify-right</property>
            <property name="group">text_align_left_radio</property>
          </object>
          <packing>
            <property name="expand">False</property>
            <property name="homogeneous">True</property>
          </packing>
        </child>
        <child>
          <object class="GtkSeparatorToolItem" id="separatortoolitem3">
            <property name="visible">True</property>
          </object>
          <packing>
            <property name="expand">False</property>
          </packing>
        </child>
        <child>
          <object class="GtkToolItem" id="toolitem3">
            <property name="visible">True</property>
            <child>
              <object class="GtkEventBox" id="text_color_eventbox">
                <property name="visible">True</property>
                <property name="tooltip_text" translatable="yes">Text color</property>
                <property name="visible_window">False</property>
                <child>
                  <placeholder/>
                </child>
              </object>
            </child>
          </object>
          <packing>
            <property name="expand">False</property>
          </packing>
        </child>
        <child>
          <object class="GtkToolItem" id="toolitem4">
            <property name="visible">True</property>
            <child>
              <object class="GtkEventBox" id="fill_color_eventbox">
                <property name="visible">True</property>
                <property name="tooltip_text" translatable="yes">Fill color</property>
                <property name="visible_window">False</property>
                <child>
                  <placeholder/>
                </child>
              </object>
            </child>
          </object>
          <packing>
            <property name="expand">False</property>
          </packing>
        </child>
        <child>
          <object class="GtkToolItem" id="toolitem5">
            <property name="visible">True</property>
            <child>
              <object class="GtkEventBox" id="line_color_eventbox">
                <property name="visible">True</property>
                <property name="tooltip_text" translatable="yes">Line color</property>
                <property name="visible_window">False</property>
                <child>
                  <placeholder/>
                </child>
              </object>
            </child>
          </object>
          <packing>
            <property name="expand">False</property>
          </packing>
        </child>
        <child>
          <object class="GtkSeparatorToolItem" id="separatortoolitem4">
            <property name="visible">True</property>
          </object>
          <packing>
            <property name="expand">False</property>
          </packing>
        </child>
        <child>
          <object class="GtkToolItem" id="toolitem6">
            <property name="visible">True</property>
            <child>
              <object class="GtkSpinButton" id="line_width_spin">
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="tooltip_text" translatable="yes">Line width</property>
                <property name="adjustment">adjustment2</property>
                <property name="climb_rate">1</property>
                <property name="digits">2</property>
              </object>
            </child>
          </object>
          <packing>
            <property name="expand">False</property>
          </packing>
        </child>
      </object>
    </child>
  </object>
  <object class="GtkAdjustment" id="adjustment1">
    <property name="value">1</property>
    <property name="lower">1</property>
    <property name="upper">250</property>
    <property name="step_increment">1</property>
    <property name="page_increment">10</property>
  </object>
  <object class="GtkAdjustment" id="adjustment2">
    <property name="value">1</property>
    <property name="lower">0.25</property>
    <property name="upper">4</property>
    <property name="step_increment">0.25</property>
    <property name="page_increment">1</property>
  </object>
</interface>
