<?xml version="1.0"?>
<interface>
  <requires lib="gtk+" version="2.16"/>
  <!-- interface-naming-policy toplevel-contextual -->
  <object class="GtkDialog" id="dialog1">
    <property name="visible">True</property>
    <property name="title" translatable="yes">gLabels Preferences</property>
    <property name="type_hint">dialog</property>
    <child internal-child="vbox">
      <object class="GtkVBox" id="dialog-vbox1">
        <property name="visible">True</property>
        <child>
          <object class="GtkNotebook" id="prefs_notebook">
            <property name="visible">True</property>
            <property name="can_focus">True</property>
            <child>
              <object class="GtkVBox" id="vbox1">
                <property name="visible">True</property>
                <property name="border_width">12</property>
                <property name="spacing">12</property>
                <child>
                  <object class="GtkLabel" id="label3">
                    <property name="visible">True</property>
                    <property name="xalign">0</property>
                    <property name="label" translatable="yes">Select locale specific behavior.</property>
                  </object>
                  <packing>
                    <property name="expand">False</property>
                    <property name="fill">False</property>
                    <property name="position">0</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkFrame" id="frame1">
                    <property name="visible">True</property>
                    <property name="label_xalign">0</property>
                    <property name="shadow_type">none</property>
                    <child>
                      <object class="GtkHBox" id="hbox1">
                        <property name="visible">True</property>
                        <child>
                          <object class="GtkLabel" id="label6">
                            <property name="visible">True</property>
                            <property name="label" translatable="yes">   </property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkVBox" id="vbox2">
                            <property name="visible">True</property>
                            <property name="border_width">6</property>
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkRadioButton" id="units_points_radio">
                                <property name="label" translatable="yes">Points</property>
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
                              <object class="GtkRadioButton" id="units_inches_radio">
                                <property name="label" translatable="yes">Inches</property>
                                <property name="visible">True</property>
                                <property name="can_focus">True</property>
                                <property name="receives_default">False</property>
                                <property name="use_underline">True</property>
                                <property name="draw_indicator">True</property>
                                <property name="group">units_points_radio</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">False</property>
                                <property name="position">1</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkRadioButton" id="units_mm_radio">
                                <property name="label" translatable="yes">Millimeters</property>
                                <property name="visible">True</property>
                                <property name="can_focus">True</property>
                                <property name="receives_default">False</property>
                                <property name="use_underline">True</property>
                                <property name="draw_indicator">True</property>
                                <property name="group">units_points_radio</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">False</property>
                                <property name="position">2</property>
                              </packing>
                            </child>
                          </object>
                          <packing>
                            <property name="position">1</property>
                          </packing>
                        </child>
                      </object>
                    </child>
                    <child type="label">
                      <object class="GtkLabel" id="label4">
                        <property name="visible">True</property>
                        <property name="xalign">0</property>
                        <property name="label" translatable="yes">&lt;span weight="bold"&gt;Units&lt;/span&gt;</property>
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
                <child>
                  <object class="GtkFrame" id="frame2">
                    <property name="visible">True</property>
                    <property name="label_xalign">0</property>
                    <property name="shadow_type">none</property>
                    <child>
                      <object class="GtkHBox" id="hbox2">
                        <property name="visible">True</property>
                        <child>
                          <object class="GtkLabel" id="label7">
                            <property name="visible">True</property>
                            <property name="label" translatable="yes">   </property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkVBox" id="vbox3">
                            <property name="visible">True</property>
                            <property name="border_width">6</property>
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkRadioButton" id="page_size_us_letter_radio">
                                <property name="label" translatable="yes">US Letter</property>
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
                              <object class="GtkRadioButton" id="page_size_a4_radio">
                                <property name="label" translatable="yes">ISO A4</property>
                                <property name="visible">True</property>
                                <property name="can_focus">True</property>
                                <property name="receives_default">False</property>
                                <property name="use_underline">True</property>
                                <property name="draw_indicator">True</property>
                                <property name="group">page_size_us_letter_radio</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">False</property>
                                <property name="position">1</property>
                              </packing>
                            </child>
                          </object>
                          <packing>
                            <property name="position">1</property>
                          </packing>
                        </child>
                      </object>
                    </child>
                    <child type="label">
                      <object class="GtkLabel" id="label5">
                        <property name="visible">True</property>
                        <property name="xalign">0</property>
                        <property name="label" translatable="yes">&lt;span weight="bold"&gt;Default page size&lt;/span&gt;</property>
                        <property name="use_markup">True</property>
                      </object>
                    </child>
                  </object>
                  <packing>
                    <property name="expand">False</property>
                    <property name="fill">False</property>
                    <property name="position">2</property>
                  </packing>
                </child>
              </object>
            </child>
            <child type="tab">
              <object class="GtkLabel" id="label1">
                <property name="visible">True</property>
                <property name="label" translatable="yes">Locale</property>
              </object>
              <packing>
                <property name="tab_fill">False</property>
              </packing>
            </child>
            <child>
              <object class="GtkVBox" id="vbox4">
                <property name="visible">True</property>
                <property name="border_width">12</property>
                <property name="spacing">12</property>
                <child>
                  <object class="GtkLabel" id="label8">
                    <property name="visible">True</property>
                    <property name="xalign">0</property>
                    <property name="label" translatable="yes">Select default properties for new objects.</property>
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
                      <object class="GtkHBox" id="hbox3">
                        <property name="visible">True</property>
                        <child>
                          <object class="GtkLabel" id="label12">
                            <property name="visible">True</property>
                            <property name="label" translatable="yes">   </property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkTable" id="table1">
                            <property name="visible">True</property>
                            <property name="n_rows">4</property>
                            <property name="n_columns">2</property>
                            <property name="column_spacing">12</property>
                            <property name="row_spacing">12</property>
                            <child>
                              <object class="GtkLabel" id="label15">
                                <property name="visible">True</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">Font:</property>
                              </object>
                              <packing>
                                <property name="x_options">GTK_FILL</property>
                                <property name="y_options"></property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkLabel" id="label16">
                                <property name="visible">True</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">Color:</property>
                              </object>
                              <packing>
                                <property name="top_attach">1</property>
                                <property name="bottom_attach">2</property>
                                <property name="x_options">GTK_FILL</property>
                                <property name="y_options"></property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkLabel" id="label17">
                                <property name="visible">True</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">Alignment:</property>
                              </object>
                              <packing>
                                <property name="top_attach">2</property>
                                <property name="bottom_attach">3</property>
                                <property name="x_options">GTK_FILL</property>
                                <property name="y_options"></property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkLabel" id="label22">
                                <property name="width_request">90</property>
                                <property name="visible">True</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">Line Spacing:</property>
                              </object>
                              <packing>
                                <property name="top_attach">3</property>
                                <property name="bottom_attach">4</property>
                                <property name="x_options">GTK_FILL</property>
                                <property name="y_options"></property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkHBox" id="hbox6">
                                <property name="visible">True</property>
                                <property name="spacing">12</property>
                                <child>
                                  <object class="GtkComboBox" id="text_family_combo">
                                    <property name="visible">True</property>
                                  </object>
                                  <packing>
                                    <property name="position">0</property>
                                  </packing>
                                </child>
                                <child>
                                  <object class="GtkSpinButton" id="text_size_spin">
                                    <property name="visible">True</property>
                                    <property name="can_focus">True</property>
                                    <property name="adjustment">adjustment1</property>
                                    <property name="climb_rate">1</property>
                                    <property name="numeric">True</property>
                                  </object>
                                  <packing>
                                    <property name="expand">False</property>
                                    <property name="fill">False</property>
                                    <property name="position">1</property>
                                  </packing>
                                </child>
                                <child>
                                  <object class="GtkToggleButton" id="text_bold_toggle">
                                    <property name="visible">True</property>
                                    <property name="can_focus">True</property>
                                    <property name="receives_default">False</property>
                                    <child>
                                      <object class="GtkImage" id="image6">
                                        <property name="visible">True</property>
                                        <property name="stock">gtk-bold</property>
                                        <property name="icon-size">4</property>
                                      </object>
                                    </child>
                                  </object>
                                  <packing>
                                    <property name="expand">False</property>
                                    <property name="fill">False</property>
                                    <property name="position">2</property>
                                  </packing>
                                </child>
                                <child>
                                  <object class="GtkToggleButton" id="text_italic_toggle">
                                    <property name="visible">True</property>
                                    <property name="can_focus">True</property>
                                    <property name="receives_default">False</property>
                                    <child>
                                      <object class="GtkImage" id="image7">
                                        <property name="visible">True</property>
                                        <property name="stock">gtk-italic</property>
                                        <property name="icon-size">4</property>
                                      </object>
                                    </child>
                                  </object>
                                  <packing>
                                    <property name="expand">False</property>
                                    <property name="fill">False</property>
                                    <property name="position">3</property>
                                  </packing>
                                </child>
                              </object>
                              <packing>
                                <property name="left_attach">1</property>
                                <property name="right_attach">2</property>
                                <property name="y_options">GTK_FILL</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkHBox" id="text_color_hbox">
                                <property name="visible">True</property>
                                <property name="spacing">12</property>
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
                                <property name="y_options">GTK_FILL</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkHBox" id="hbox8">
                                <property name="visible">True</property>
                                <property name="spacing">12</property>
                                <child>
                                  <object class="GtkToggleButton" id="text_left_toggle">
                                    <property name="visible">True</property>
                                    <property name="can_focus">True</property>
                                    <property name="receives_default">False</property>
                                    <child>
                                      <object class="GtkImage" id="image3">
                                        <property name="visible">True</property>
                                        <property name="stock">gtk-justify-left</property>
                                        <property name="icon-size">4</property>
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
                                  <object class="GtkToggleButton" id="text_center_toggle">
                                    <property name="visible">True</property>
                                    <property name="can_focus">True</property>
                                    <property name="receives_default">False</property>
                                    <child>
                                      <object class="GtkImage" id="image4">
                                        <property name="visible">True</property>
                                        <property name="stock">gtk-justify-center</property>
                                        <property name="icon-size">4</property>
                                      </object>
                                    </child>
                                  </object>
                                  <packing>
                                    <property name="expand">False</property>
                                    <property name="fill">False</property>
                                    <property name="position">1</property>
                                  </packing>
                                </child>
                                <child>
                                  <object class="GtkToggleButton" id="text_right_toggle">
                                    <property name="visible">True</property>
                                    <property name="can_focus">True</property>
                                    <property name="receives_default">False</property>
                                    <child>
                                      <object class="GtkImage" id="image5">
                                        <property name="visible">True</property>
                                        <property name="stock">gtk-justify-right</property>
                                        <property name="icon-size">4</property>
                                      </object>
                                    </child>
                                  </object>
                                  <packing>
                                    <property name="expand">False</property>
                                    <property name="fill">False</property>
                                    <property name="position">2</property>
                                  </packing>
                                </child>
                              </object>
                              <packing>
                                <property name="left_attach">1</property>
                                <property name="right_attach">2</property>
                                <property name="top_attach">2</property>
                                <property name="bottom_attach">3</property>
                                <property name="x_options">GTK_FILL</property>
                                <property name="y_options">GTK_FILL</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkHBox" id="hbox12">
                                <property name="visible">True</property>
                                <property name="spacing">40</property>
                                <child>
                                  <object class="GtkSpinButton" id="text_line_spacing_spin">
                                    <property name="visible">True</property>
                                    <property name="can_focus">True</property>
                                    <property name="adjustment">adjustment2</property>
                                    <property name="climb_rate">0.0099999997764800008</property>
                                    <property name="digits">2</property>
                                  </object>
                                  <packing>
                                    <property name="expand">False</property>
                                    <property name="fill">False</property>
                                    <property name="position">0</property>
                                  </packing>
                                </child>
                              </object>
                              <packing>
                                <property name="left_attach">1</property>
                                <property name="right_attach">2</property>
                                <property name="top_attach">3</property>
                                <property name="bottom_attach">4</property>
                                <property name="x_options">GTK_FILL</property>
                                <property name="y_options">GTK_FILL</property>
                              </packing>
                            </child>
                          </object>
                          <packing>
                            <property name="position">1</property>
                          </packing>
                        </child>
                      </object>
                    </child>
                    <child type="label">
                      <object class="GtkLabel" id="label9">
                        <property name="visible">True</property>
                        <property name="label" translatable="yes">&lt;span weight="bold"&gt;Text&lt;/span&gt;</property>
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
                <child>
                  <object class="GtkFrame" id="frame4">
                    <property name="visible">True</property>
                    <property name="label_xalign">0</property>
                    <property name="shadow_type">none</property>
                    <child>
                      <object class="GtkHBox" id="hbox4">
                        <property name="visible">True</property>
                        <child>
                          <object class="GtkLabel" id="label13">
                            <property name="visible">True</property>
                            <property name="label">   </property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkTable" id="table2">
                            <property name="visible">True</property>
                            <property name="n_rows">2</property>
                            <property name="n_columns">2</property>
                            <property name="column_spacing">12</property>
                            <property name="row_spacing">12</property>
                            <child>
                              <object class="GtkLabel" id="label18">
                                <property name="width_request">90</property>
                                <property name="visible">True</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">Width:</property>
                              </object>
                              <packing>
                                <property name="x_options">GTK_FILL</property>
                                <property name="y_options"></property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkLabel" id="label19">
                                <property name="visible">True</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">Color:</property>
                              </object>
                              <packing>
                                <property name="top_attach">1</property>
                                <property name="bottom_attach">2</property>
                                <property name="x_options">GTK_FILL</property>
                                <property name="y_options"></property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkHBox" id="hbox9">
                                <property name="visible">True</property>
                                <property name="spacing">12</property>
                                <child>
                                  <object class="GtkSpinButton" id="line_width_spin">
                                    <property name="visible">True</property>
                                    <property name="can_focus">True</property>
                                    <property name="adjustment">adjustment3</property>
                                    <property name="climb_rate">0.0099999997764800008</property>
                                    <property name="digits">2</property>
                                    <property name="numeric">True</property>
                                  </object>
                                  <packing>
                                    <property name="expand">False</property>
                                    <property name="fill">False</property>
                                    <property name="position">0</property>
                                  </packing>
                                </child>
                                <child>
                                  <object class="GtkLabel" id="label21">
                                    <property name="visible">True</property>
                                    <property name="label" translatable="yes">points</property>
                                  </object>
                                  <packing>
                                    <property name="expand">False</property>
                                    <property name="fill">False</property>
                                    <property name="position">1</property>
                                  </packing>
                                </child>
                              </object>
                              <packing>
                                <property name="left_attach">1</property>
                                <property name="right_attach">2</property>
                                <property name="y_options">GTK_FILL</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkHBox" id="line_color_hbox">
                                <property name="visible">True</property>
                                <property name="spacing">12</property>
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
                                <property name="y_options">GTK_FILL</property>
                              </packing>
                            </child>
                          </object>
                          <packing>
                            <property name="position">1</property>
                          </packing>
                        </child>
                      </object>
                    </child>
                    <child type="label">
                      <object class="GtkLabel" id="label10">
                        <property name="visible">True</property>
                        <property name="label" translatable="yes">&lt;span weight="bold"&gt;Line&lt;/span&gt;</property>
                        <property name="use_markup">True</property>
                      </object>
                    </child>
                  </object>
                  <packing>
                    <property name="expand">False</property>
                    <property name="fill">False</property>
                    <property name="position">2</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkFrame" id="frame5">
                    <property name="visible">True</property>
                    <property name="label_xalign">0</property>
                    <property name="shadow_type">none</property>
                    <child>
                      <object class="GtkHBox" id="hbox5">
                        <property name="visible">True</property>
                        <child>
                          <object class="GtkLabel" id="label14">
                            <property name="visible">True</property>
                            <property name="label" translatable="yes">   </property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkTable" id="table4">
                            <property name="visible">True</property>
                            <property name="n_columns">2</property>
                            <property name="column_spacing">12</property>
                            <property name="row_spacing">12</property>
                            <child>
                              <object class="GtkLabel" id="label20">
                                <property name="width_request">90</property>
                                <property name="visible">True</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">Color:</property>
                              </object>
                              <packing>
                                <property name="x_options">GTK_FILL</property>
                                <property name="y_options"></property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkHBox" id="fill_color_hbox">
                                <property name="visible">True</property>
                                <property name="spacing">12</property>
                                <child>
                                  <placeholder/>
                                </child>
                              </object>
                              <packing>
                                <property name="left_attach">1</property>
                                <property name="right_attach">2</property>
                                <property name="y_options">GTK_FILL</property>
                              </packing>
                            </child>
                          </object>
                          <packing>
                            <property name="position">1</property>
                          </packing>
                        </child>
                      </object>
                    </child>
                    <child type="label">
                      <object class="GtkLabel" id="label11">
                        <property name="visible">True</property>
                        <property name="label" translatable="yes">&lt;span weight="bold"&gt;Fill&lt;/span&gt;</property>
                        <property name="use_markup">True</property>
                      </object>
                    </child>
                  </object>
                  <packing>
                    <property name="expand">False</property>
                    <property name="fill">False</property>
                    <property name="position">3</property>
                  </packing>
                </child>
              </object>
              <packing>
                <property name="position">1</property>
              </packing>
            </child>
            <child type="tab">
              <object class="GtkLabel" id="label2">
                <property name="visible">True</property>
                <property name="label" translatable="yes">Object defaults</property>
              </object>
              <packing>
                <property name="position">1</property>
                <property name="tab_fill">False</property>
              </packing>
            </child>
          </object>
          <packing>
            <property name="position">2</property>
          </packing>
        </child>
        <child internal-child="action_area">
          <object class="GtkHButtonBox" id="dialog-action_area1">
            <property name="visible">True</property>
            <property name="layout_style">end</property>
            <child>
              <object class="GtkButton" id="closebutton1">
                <property name="label">gtk-close</property>
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="can_default">True</property>
                <property name="receives_default">False</property>
                <property name="use_stock">True</property>
              </object>
              <packing>
                <property name="expand">False</property>
                <property name="fill">False</property>
                <property name="position">0</property>
              </packing>
            </child>
          </object>
          <packing>
            <property name="expand">False</property>
            <property name="pack_type">end</property>
            <property name="position">0</property>
          </packing>
        </child>
      </object>
    </child>
    <action-widgets>
      <action-widget response="-7">closebutton1</action-widget>
    </action-widgets>
  </object>
  <object class="GtkAdjustment" id="adjustment1">
    <property name="value">1</property>
    <property name="lower">1</property>
    <property name="upper">100</property>
    <property name="step_increment">1</property>
    <property name="page_increment">10</property>
  </object>
  <object class="GtkAdjustment" id="adjustment2">
    <property name="value">1</property>
    <property name="upper">5</property>
    <property name="step_increment">0.01</property>
    <property name="page_increment">0.10000000000000001</property>
  </object>
  <object class="GtkAdjustment" id="adjustment3">
    <property name="value">1</property>
    <property name="lower">0.25</property>
    <property name="upper">4</property>
    <property name="step_increment">0.25</property>
    <property name="page_increment">1</property>
  </object>
</interface>
