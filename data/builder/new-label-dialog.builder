<?xml version="1.0"?>
<interface>
  <requires lib="gtk+" version="2.16"/>
  <!-- interface-naming-policy project-wide -->
  <object class="GtkWindow" id="window1">
    <child>
      <object class="GtkHBox" id="new_label_dialog_hbox">
        <property name="visible">True</property>
        <property name="border_width">6</property>
        <property name="spacing">18</property>
        <child>
          <object class="GtkVBox" id="vbox1">
            <property name="visible">True</property>
            <property name="orientation">vertical</property>
            <property name="spacing">6</property>
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
                      <object class="GtkTable" id="table2">
                        <property name="visible">True</property>
                        <property name="n_rows">2</property>
                        <property name="n_columns">2</property>
                        <property name="column_spacing">12</property>
                        <property name="row_spacing">6</property>
                        <child>
                          <object class="GtkLabel" id="label6">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">Template:</property>
                          </object>
                          <packing>
                            <property name="x_options">GTK_FILL</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkHBox" id="combo_hbox">
                            <property name="visible">True</property>
                            <child>
                              <placeholder/>
                            </child>
                          </object>
                          <packing>
                            <property name="left_attach">1</property>
                            <property name="right_attach">2</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkLabel" id="label12">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="yalign">0</property>
                            <property name="label" translatable="yes">Orientation:</property>
                          </object>
                          <packing>
                            <property name="top_attach">1</property>
                            <property name="bottom_attach">2</property>
                            <property name="x_options">GTK_FILL</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkHBox" id="rotate_hbox">
                            <property name="visible">True</property>
                            <child>
                              <placeholder/>
                            </child>
                          </object>
                          <packing>
                            <property name="left_attach">1</property>
                            <property name="right_attach">2</property>
                            <property name="top_attach">1</property>
                            <property name="bottom_attach">2</property>
                            <property name="x_options">GTK_FILL</property>
                          </packing>
                        </child>
                      </object>
                    </child>
                  </object>
                </child>
                <child type="label">
                  <object class="GtkLabel" id="label9">
                    <property name="visible">True</property>
                    <property name="label" translatable="yes">&lt;b&gt;Select media&lt;/b&gt;</property>
                    <property name="use_markup">True</property>
                  </object>
                </child>
              </object>
              <packing>
                <property name="expand">False</property>
                <property name="fill">False</property>
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
                      <object class="GtkTable" id="table1">
                        <property name="visible">True</property>
                        <property name="n_rows">7</property>
                        <property name="n_columns">2</property>
                        <property name="column_spacing">12</property>
                        <property name="row_spacing">6</property>
                        <child>
                          <object class="GtkLabel" id="label1">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">Description:</property>
                          </object>
                          <packing>
                            <property name="top_attach">2</property>
                            <property name="bottom_attach">3</property>
                            <property name="x_options">GTK_FILL</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkLabel" id="label2">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">Page size:</property>
                          </object>
                          <packing>
                            <property name="top_attach">3</property>
                            <property name="bottom_attach">4</property>
                            <property name="x_options">GTK_FILL</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkLabel" id="label3">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">Label size:</property>
                          </object>
                          <packing>
                            <property name="top_attach">4</property>
                            <property name="bottom_attach">5</property>
                            <property name="x_options">GTK_FILL</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkLabel" id="label4">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">Layout:</property>
                          </object>
                          <packing>
                            <property name="top_attach">5</property>
                            <property name="bottom_attach">6</property>
                            <property name="x_options">GTK_FILL</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkLabel" id="label7">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="yalign">0</property>
                            <property name="label" translatable="yes">Similar products:</property>
                          </object>
                          <packing>
                            <property name="top_attach">6</property>
                            <property name="bottom_attach">7</property>
                            <property name="x_options">GTK_FILL</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkLabel" id="desc_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                          </object>
                          <packing>
                            <property name="left_attach">1</property>
                            <property name="right_attach">2</property>
                            <property name="top_attach">2</property>
                            <property name="bottom_attach">3</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkLabel" id="page_size_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                          </object>
                          <packing>
                            <property name="left_attach">1</property>
                            <property name="right_attach">2</property>
                            <property name="top_attach">3</property>
                            <property name="bottom_attach">4</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkLabel" id="label_size_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                          </object>
                          <packing>
                            <property name="left_attach">1</property>
                            <property name="right_attach">2</property>
                            <property name="top_attach">4</property>
                            <property name="bottom_attach">5</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkLabel" id="layout_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                          </object>
                          <packing>
                            <property name="left_attach">1</property>
                            <property name="right_attach">2</property>
                            <property name="top_attach">5</property>
                            <property name="bottom_attach">6</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkLabel" id="label5">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">Vendor:</property>
                          </object>
                          <packing>
                            <property name="x_options">GTK_FILL</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkLabel" id="vendor_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                          </object>
                          <packing>
                            <property name="left_attach">1</property>
                            <property name="right_attach">2</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkLabel" id="label8">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">Part #:</property>
                          </object>
                          <packing>
                            <property name="top_attach">1</property>
                            <property name="bottom_attach">2</property>
                            <property name="x_options">GTK_FILL</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkLabel" id="part_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                          </object>
                          <packing>
                            <property name="left_attach">1</property>
                            <property name="right_attach">2</property>
                            <property name="top_attach">1</property>
                            <property name="bottom_attach">2</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkScrolledWindow" id="scrolledwindow1">
                            <property name="height_request">77</property>
                            <property name="visible">True</property>
                            <property name="can_focus">True</property>
                            <property name="hscrollbar_policy">never</property>
                            <property name="vscrollbar_policy">automatic</property>
                            <property name="shadow_type">in</property>
                            <child>
                              <object class="GtkViewport" id="viewport1">
                                <property name="visible">True</property>
                                <property name="resize_mode">queue</property>
                                <property name="shadow_type">none</property>
                                <child>
                                  <object class="GtkLabel" id="similar_label">
                                    <property name="visible">True</property>
                                    <property name="xalign">0</property>
                                    <property name="yalign">0</property>
                                    <property name="label" translatable="yes">1
2
3
4</property>
                                  </object>
                                </child>
                              </object>
                            </child>
                          </object>
                          <packing>
                            <property name="left_attach">1</property>
                            <property name="right_attach">2</property>
                            <property name="top_attach">6</property>
                            <property name="bottom_attach">7</property>
                            <property name="y_options"></property>
                          </packing>
                        </child>
                      </object>
                    </child>
                  </object>
                </child>
                <child type="label">
                  <object class="GtkLabel" id="label10">
                    <property name="visible">True</property>
                    <property name="label" translatable="yes">&lt;b&gt;Information&lt;/b&gt;</property>
                    <property name="use_markup">True</property>
                  </object>
                </child>
              </object>
              <packing>
                <property name="expand">False</property>
                <property name="fill">False</property>
                <property name="position">1</property>
              </packing>
            </child>
          </object>
          <packing>
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
                  <object class="GtkVBox" id="preview_vbox">
                    <property name="visible">True</property>
                    <property name="orientation">vertical</property>
                    <child>
                      <placeholder/>
                    </child>
                  </object>
                </child>
              </object>
            </child>
            <child type="label">
              <object class="GtkLabel" id="label11">
                <property name="visible">True</property>
                <property name="label" translatable="yes">&lt;b&gt;Preview&lt;/b&gt;</property>
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
