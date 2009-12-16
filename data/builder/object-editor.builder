<?xml version="1.0"?>
<interface>
  <requires lib="gtk+" version="2.16"/>
  <!-- interface-naming-policy toplevel-contextual -->
  <object class="GtkDialog" id="dialog">
    <property name="visible">True</property>
    <property name="border_width">6</property>
    <property name="title" translatable="yes">dialog1</property>
    <property name="type_hint">dialog</property>
    <child internal-child="vbox">
      <object class="GtkVBox" id="dialog-vbox1">
        <property name="visible">True</property>
        <property name="orientation">vertical</property>
        <child>
          <object class="GtkVBox" id="editor_vbox">
            <property name="visible">True</property>
            <property name="border_width">6</property>
            <property name="orientation">vertical</property>
            <property name="spacing">12</property>
            <child>
              <object class="GtkHBox" id="title_hbox">
                <property name="visible">True</property>
                <property name="spacing">12</property>
                <child>
                  <object class="GtkImage" id="title_image">
                    <property name="visible">True</property>
                  </object>
                  <packing>
                    <property name="expand">False</property>
                    <property name="position">0</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkLabel" id="title_label">
                    <property name="visible">True</property>
                    <property name="label" translatable="yes">&lt;span weight="bold" size="larger"&gt;Xxx object properties&lt;/span&gt;</property>
                    <property name="use_markup">True</property>
                  </object>
                  <packing>
                    <property name="expand">False</property>
                    <property name="fill">False</property>
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
              <object class="GtkNotebook" id="notebook">
                <property name="visible">True</property>
                <property name="can_focus">True</property>
                <property name="scrollable">True</property>
                <property name="homogeneous">True</property>
                <child>
                  <object class="GtkVBox" id="edit_page_vbox">
                    <property name="visible">True</property>
                    <property name="border_width">12</property>
                    <property name="orientation">vertical</property>
                    <property name="spacing">6</property>
                    <child>
                      <object class="GtkHBox" id="hbox30">
                        <property name="visible">True</property>
                        <child>
                          <object class="GtkScrolledWindow" id="scrolledwindow1">
                            <property name="visible">True</property>
                            <property name="can_focus">True</property>
                            <child>
                              <object class="GtkTextView" id="edit_text_view">
                                <property name="width_request">232</property>
                                <property name="visible">True</property>
                                <property name="can_focus">True</property>
                              </object>
                            </child>
                          </object>
                          <packing>
                            <property name="position">0</property>
                          </packing>
                        </child>
                      </object>
                      <packing>
                        <property name="position">0</property>
                      </packing>
                    </child>
                    <child>
                      <object class="GtkHBox" id="hbox1">
                        <property name="visible">True</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkVBox" id="edit_insert_field_vbox">
                            <property name="visible">True</property>
                            <property name="orientation">vertical</property>
                            <child>
                              <placeholder/>
                            </child>
                          </object>
                          <packing>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
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
                    <property name="tab_fill">False</property>
                  </packing>
                </child>
                <child type="tab">
                  <object class="GtkLabel" id="edit_tab_label">
                    <property name="visible">True</property>
                    <property name="label" translatable="yes">Text</property>
                  </object>
                  <packing>
                    <property name="tab_fill">False</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkVBox" id="text_page_vbox">
                    <property name="visible">True</property>
                    <property name="border_width">12</property>
                    <property name="orientation">vertical</property>
                    <property name="spacing">12</property>
                    <child>
                      <object class="GtkHBox" id="hbox65">
                        <property name="visible">True</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkLabel" id="text_family_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">Family:</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkHBox" id="text_family_hbox">
                            <property name="visible">True</property>
                            <child>
                              <placeholder/>
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
                      <object class="GtkHBox" id="hbox66">
                        <property name="visible">True</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkLabel" id="text_size_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">Size:</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkHBox" id="hbox28">
                            <property name="visible">True</property>
                            <property name="spacing">12</property>
                            <child>
                              <object class="GtkSpinButton" id="text_size_spin">
                                <property name="visible">True</property>
                                <property name="can_focus">True</property>
                                <property name="adjustment">adjustment1</property>
                                <property name="climb_rate">0.0099999997764800008</property>
                                <property name="numeric">True</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">False</property>
                                <property name="position">0</property>
                              </packing>
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
                        <property name="position">1</property>
                      </packing>
                    </child>
                    <child>
                      <object class="GtkHBox" id="hbox67">
                        <property name="visible">True</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkLabel" id="text_style_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">Style:</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkHBox" id="hbox29">
                            <property name="visible">True</property>
                            <property name="spacing">12</property>
                            <child>
                              <object class="GtkToggleButton" id="text_bold_toggle">
                                <property name="visible">True</property>
                                <property name="can_focus">True</property>
                                <property name="receives_default">False</property>
                                <child>
                                  <object class="GtkImage" id="image1">
                                    <property name="visible">True</property>
                                    <property name="stock">gtk-bold</property>
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
                              <object class="GtkToggleButton" id="text_italic_toggle">
                                <property name="visible">True</property>
                                <property name="can_focus">True</property>
                                <property name="receives_default">False</property>
                                <child>
                                  <object class="GtkImage" id="image2">
                                    <property name="visible">True</property>
                                    <property name="stock">gtk-italic</property>
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
                            <property name="position">1</property>
                          </packing>
                        </child>
                      </object>
                      <packing>
                        <property name="expand">False</property>
                        <property name="fill">False</property>
                        <property name="position">2</property>
                      </packing>
                    </child>
                    <child>
                      <object class="GtkHBox" id="hbox68">
                        <property name="visible">True</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkLabel" id="text_color_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="yalign">0.14000000059604645</property>
                            <property name="label" translatable="yes">Color:</property>
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
                            <property name="orientation">vertical</property>
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkHBox" id="hbox42">
                                <property name="visible">True</property>
                                <property name="spacing">6</property>
                                <child>
                                  <object class="GtkRadioButton" id="text_color_radio">
                                    <property name="visible">True</property>
                                    <property name="can_focus">True</property>
                                    <property name="receives_default">False</property>
                                    <property name="use_underline">True</property>
                                    <property name="focus_on_click">False</property>
                                    <property name="draw_indicator">True</property>
                                  </object>
                                  <packing>
                                    <property name="expand">False</property>
                                    <property name="fill">False</property>
                                    <property name="position">0</property>
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
                                    <property name="position">1</property>
                                  </packing>
                                </child>
                              </object>
                              <packing>
                                <property name="position">0</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkHBox" id="hbox44">
                                <property name="visible">True</property>
                                <property name="spacing">6</property>
                                <child>
                                  <object class="GtkRadioButton" id="text_color_key_radio">
                                    <property name="label" translatable="yes">Key:</property>
                                    <property name="visible">True</property>
                                    <property name="can_focus">True</property>
                                    <property name="receives_default">False</property>
                                    <property name="use_underline">True</property>
                                    <property name="focus_on_click">False</property>
                                    <property name="draw_indicator">True</property>
                                    <property name="group">text_color_radio</property>
                                  </object>
                                  <packing>
                                    <property name="expand">False</property>
                                    <property name="fill">False</property>
                                    <property name="position">0</property>
                                  </packing>
                                </child>
                                <child>
                                  <object class="GtkHBox" id="text_color_key_hbox">
                                    <property name="visible">True</property>
                                    <property name="spacing">12</property>
                                    <child>
                                      <placeholder/>
                                    </child>
                                  </object>
                                  <packing>
                                    <property name="position">1</property>
                                  </packing>
                                </child>
                              </object>
                              <packing>
                                <property name="position">1</property>
                              </packing>
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
                        <property name="position">3</property>
                      </packing>
                    </child>
                    <child>
                      <object class="GtkHBox" id="hbox69">
                        <property name="visible">True</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkLabel" id="text_align_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">Alignment:</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkHBox" id="hbox4">
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
                            <property name="position">1</property>
                          </packing>
                        </child>
                      </object>
                      <packing>
                        <property name="expand">False</property>
                        <property name="fill">False</property>
                        <property name="position">4</property>
                      </packing>
                    </child>
                    <child>
                      <object class="GtkHBox" id="hbox70">
                        <property name="visible">True</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkLabel" id="text_line_spacing_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">Line Spacing:</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkHBox" id="hbox31">
                            <property name="visible">True</property>
                            <property name="spacing">12</property>
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
                            <property name="position">1</property>
                          </packing>
                        </child>
                      </object>
                      <packing>
                        <property name="expand">False</property>
                        <property name="fill">False</property>
                        <property name="position">5</property>
                      </packing>
                    </child>
                    <child>
                      <object class="GtkCheckButton" id="text_auto_shrink_check">
                        <property name="label" translatable="yes">Allow merge to automatically shrink text</property>
                        <property name="visible">True</property>
                        <property name="can_focus">True</property>
                        <property name="receives_default">False</property>
                        <property name="use_underline">True</property>
                        <property name="draw_indicator">True</property>
                      </object>
                      <packing>
                        <property name="expand">False</property>
                        <property name="fill">False</property>
                        <property name="position">6</property>
                      </packing>
                    </child>
                  </object>
                  <packing>
                    <property name="position">1</property>
                    <property name="tab_fill">False</property>
                  </packing>
                </child>
                <child type="tab">
                  <object class="GtkLabel" id="text_tab_label">
                    <property name="visible">True</property>
                    <property name="label" translatable="yes">Style</property>
                  </object>
                  <packing>
                    <property name="position">1</property>
                    <property name="tab_fill">False</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkVBox" id="line_page_vbox">
                    <property name="visible">True</property>
                    <property name="border_width">12</property>
                    <property name="orientation">vertical</property>
                    <property name="spacing">12</property>
                    <child>
                      <object class="GtkHBox" id="hbox71">
                        <property name="visible">True</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkLabel" id="line_w_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">Width:</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkHBox" id="hbox7">
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
                      <object class="GtkHBox" id="hbox72">
                        <property name="visible">True</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkLabel" id="line_color_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="yalign">0.14000000059604645</property>
                            <property name="label" translatable="yes">Color:</property>
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
                            <property name="orientation">vertical</property>
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkHBox" id="hbox38">
                                <property name="visible">True</property>
                                <property name="spacing">6</property>
                                <child>
                                  <object class="GtkRadioButton" id="line_color_radio">
                                    <property name="visible">True</property>
                                    <property name="can_focus">False</property>
                                    <property name="receives_default">False</property>
                                    <property name="use_underline">True</property>
                                    <property name="focus_on_click">False</property>
                                    <property name="draw_indicator">True</property>
                                  </object>
                                  <packing>
                                    <property name="expand">False</property>
                                    <property name="fill">False</property>
                                    <property name="position">0</property>
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
                                    <property name="position">1</property>
                                  </packing>
                                </child>
                              </object>
                              <packing>
                                <property name="position">0</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkHBox" id="hbox40">
                                <property name="visible">True</property>
                                <property name="spacing">6</property>
                                <child>
                                  <object class="GtkRadioButton" id="line_key_radio">
                                    <property name="label" translatable="yes">Key:</property>
                                    <property name="visible">True</property>
                                    <property name="can_focus">False</property>
                                    <property name="receives_default">False</property>
                                    <property name="use_underline">True</property>
                                    <property name="focus_on_click">False</property>
                                    <property name="draw_indicator">True</property>
                                    <property name="group">line_color_radio</property>
                                  </object>
                                  <packing>
                                    <property name="expand">False</property>
                                    <property name="fill">False</property>
                                    <property name="position">0</property>
                                  </packing>
                                </child>
                                <child>
                                  <object class="GtkHBox" id="line_key_hbox">
                                    <property name="visible">True</property>
                                    <property name="spacing">12</property>
                                    <child>
                                      <placeholder/>
                                    </child>
                                  </object>
                                  <packing>
                                    <property name="position">1</property>
                                  </packing>
                                </child>
                              </object>
                              <packing>
                                <property name="position">1</property>
                              </packing>
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
                        <property name="position">1</property>
                      </packing>
                    </child>
                  </object>
                  <packing>
                    <property name="position">2</property>
                  </packing>
                </child>
                <child type="tab">
                  <object class="GtkLabel" id="line_tab_label">
                    <property name="visible">True</property>
                    <property name="label" translatable="yes">Line</property>
                  </object>
                  <packing>
                    <property name="position">2</property>
                    <property name="tab_fill">False</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkVBox" id="fill_page_vbox">
                    <property name="visible">True</property>
                    <property name="border_width">12</property>
                    <property name="orientation">vertical</property>
                    <child>
                      <object class="GtkHBox" id="hbox73">
                        <property name="visible">True</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkLabel" id="fill_color_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="yalign">0.14000000059604645</property>
                            <property name="label" translatable="yes">Color:</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkVBox" id="vbox5">
                            <property name="visible">True</property>
                            <property name="orientation">vertical</property>
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkHBox" id="hbox50">
                                <property name="visible">True</property>
                                <property name="spacing">6</property>
                                <child>
                                  <object class="GtkRadioButton" id="fill_color_radio">
                                    <property name="visible">True</property>
                                    <property name="can_focus">False</property>
                                    <property name="receives_default">False</property>
                                    <property name="use_underline">True</property>
                                    <property name="focus_on_click">False</property>
                                    <property name="active">True</property>
                                    <property name="draw_indicator">True</property>
                                  </object>
                                  <packing>
                                    <property name="expand">False</property>
                                    <property name="fill">False</property>
                                    <property name="position">0</property>
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
                                    <property name="position">1</property>
                                  </packing>
                                </child>
                              </object>
                              <packing>
                                <property name="position">0</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkHBox" id="hbox52">
                                <property name="visible">True</property>
                                <property name="spacing">6</property>
                                <child>
                                  <object class="GtkRadioButton" id="fill_key_radio">
                                    <property name="label" translatable="yes">Key:</property>
                                    <property name="visible">True</property>
                                    <property name="can_focus">False</property>
                                    <property name="receives_default">False</property>
                                    <property name="use_underline">True</property>
                                    <property name="focus_on_click">False</property>
                                    <property name="draw_indicator">True</property>
                                    <property name="group">fill_color_radio</property>
                                  </object>
                                  <packing>
                                    <property name="expand">False</property>
                                    <property name="fill">False</property>
                                    <property name="position">0</property>
                                  </packing>
                                </child>
                                <child>
                                  <object class="GtkHBox" id="fill_key_hbox">
                                    <property name="visible">True</property>
                                    <property name="spacing">12</property>
                                    <child>
                                      <placeholder/>
                                    </child>
                                  </object>
                                  <packing>
                                    <property name="position">1</property>
                                  </packing>
                                </child>
                              </object>
                              <packing>
                                <property name="position">1</property>
                              </packing>
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
                  </object>
                  <packing>
                    <property name="position">3</property>
                  </packing>
                </child>
                <child type="tab">
                  <object class="GtkLabel" id="fill_tab_label">
                    <property name="visible">True</property>
                    <property name="label" translatable="yes">Fill</property>
                  </object>
                  <packing>
                    <property name="position">3</property>
                    <property name="tab_fill">False</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkVBox" id="img_page_vbox">
                    <property name="visible">True</property>
                    <property name="border_width">12</property>
                    <property name="orientation">vertical</property>
                    <property name="spacing">12</property>
                    <child>
                      <object class="GtkHBox" id="hbox74">
                        <property name="visible">True</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkRadioButton" id="img_file_radio">
                            <property name="label" translatable="yes">File:</property>
                            <property name="visible">True</property>
                            <property name="can_focus">True</property>
                            <property name="receives_default">False</property>
                            <property name="use_underline">True</property>
                            <property name="focus_on_click">False</property>
                            <property name="draw_indicator">True</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkFileChooserButton" id="img_file_button">
                            <property name="visible">True</property>
                            <property name="show_hidden">True</property>
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
                      <object class="GtkHBox" id="hbox75">
                        <property name="visible">True</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkRadioButton" id="img_key_radio">
                            <property name="label" translatable="yes">Key:</property>
                            <property name="visible">True</property>
                            <property name="sensitive">False</property>
                            <property name="can_focus">True</property>
                            <property name="receives_default">False</property>
                            <property name="use_underline">True</property>
                            <property name="focus_on_click">False</property>
                            <property name="draw_indicator">True</property>
                            <property name="group">img_file_radio</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkHBox" id="img_key_hbox">
                            <property name="visible">True</property>
                            <child>
                              <placeholder/>
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
                        <property name="position">1</property>
                      </packing>
                    </child>
                  </object>
                  <packing>
                    <property name="position">4</property>
                  </packing>
                </child>
                <child type="tab">
                  <object class="GtkLabel" id="img_tab_label">
                    <property name="visible">True</property>
                    <property name="label" translatable="yes">Image</property>
                  </object>
                  <packing>
                    <property name="position">4</property>
                    <property name="tab_fill">False</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkVBox" id="data_page_vbox">
                    <property name="visible">True</property>
                    <property name="border_width">12</property>
                    <property name="orientation">vertical</property>
                    <property name="spacing">12</property>
                    <child>
                      <object class="GtkHBox" id="hbox76">
                        <property name="visible">True</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkRadioButton" id="data_literal_radio">
                            <property name="label" translatable="yes">Literal:</property>
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
                          <object class="GtkEntry" id="data_text_entry">
                            <property name="visible">True</property>
                            <property name="can_focus">True</property>
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
                      <object class="GtkHBox" id="hbox77">
                        <property name="visible">True</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkRadioButton" id="data_key_radio">
                            <property name="label" translatable="yes">Key:</property>
                            <property name="visible">True</property>
                            <property name="can_focus">True</property>
                            <property name="receives_default">False</property>
                            <property name="use_underline">True</property>
                            <property name="draw_indicator">True</property>
                            <property name="group">data_literal_radio</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkHBox" id="data_key_hbox">
                            <property name="visible">True</property>
                            <child>
                              <placeholder/>
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
                        <property name="position">1</property>
                      </packing>
                    </child>
                    <child>
                      <object class="GtkHBox" id="hbox78">
                        <property name="visible">True</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkLabel" id="data_fill_label">
                            <property name="visible">True</property>
                            <property name="label">         </property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkTable" id="table9">
                            <property name="visible">True</property>
                            <property name="n_rows">2</property>
                            <property name="n_columns">2</property>
                            <property name="column_spacing">6</property>
                            <property name="row_spacing">6</property>
                            <child>
                              <object class="GtkLabel" id="data_format_label">
                                <property name="visible">True</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">format:</property>
                              </object>
                              <packing>
                                <property name="x_options">GTK_FILL</property>
                                <property name="y_options"></property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkLabel" id="data_ex_label">
                                <property name="visible">True</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">00000000000 00000</property>
                              </object>
                              <packing>
                                <property name="left_attach">1</property>
                                <property name="right_attach">2</property>
                                <property name="x_options">GTK_FILL</property>
                                <property name="y_options"></property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkLabel" id="data_digits_label">
                                <property name="visible">True</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">digits:</property>
                              </object>
                              <packing>
                                <property name="top_attach">1</property>
                                <property name="bottom_attach">2</property>
                                <property name="x_options">GTK_FILL</property>
                                <property name="y_options"></property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkHBox" id="hbox32">
                                <property name="visible">True</property>
                                <child>
                                  <object class="GtkSpinButton" id="data_digits_spin">
                                    <property name="visible">True</property>
                                    <property name="can_focus">True</property>
                                    <property name="adjustment">adjustment13</property>
                                    <property name="climb_rate">1</property>
                                    <property name="numeric">True</property>
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
                      <packing>
                        <property name="expand">False</property>
                        <property name="fill">False</property>
                        <property name="position">2</property>
                      </packing>
                    </child>
                  </object>
                  <packing>
                    <property name="position">5</property>
                  </packing>
                </child>
                <child type="tab">
                  <object class="GtkLabel" id="data_tab_label">
                    <property name="visible">True</property>
                    <property name="label" translatable="yes">Data</property>
                  </object>
                  <packing>
                    <property name="position">5</property>
                    <property name="tab_fill">False</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkVBox" id="bc_page_vbox">
                    <property name="visible">True</property>
                    <property name="border_width">12</property>
                    <property name="orientation">vertical</property>
                    <property name="spacing">12</property>
                    <child>
                      <object class="GtkHBox" id="hbox79">
                        <property name="visible">True</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkLabel" id="bc_style_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">Style:</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkComboBox" id="bc_style_combo">
                            <property name="visible">True</property>
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
                      <object class="GtkCheckButton" id="bc_text_check">
                        <property name="label" translatable="yes">Text</property>
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
                      <object class="GtkCheckButton" id="bc_cs_check">
                        <property name="label" translatable="yes">Checksum</property>
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
                    <child>
                      <object class="GtkHBox" id="hbox80">
                        <property name="visible">True</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkLabel" id="bc_color_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="yalign">0.14000000059604645</property>
                            <property name="label" translatable="yes">Color:</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkVBox" id="vbox4">
                            <property name="visible">True</property>
                            <property name="orientation">vertical</property>
                            <property name="spacing">6</property>
                            <child>
                              <object class="GtkHBox" id="hbox46">
                                <property name="visible">True</property>
                                <property name="spacing">6</property>
                                <child>
                                  <object class="GtkRadioButton" id="bc_color_radio">
                                    <property name="visible">True</property>
                                    <property name="can_focus">True</property>
                                    <property name="receives_default">False</property>
                                    <property name="use_underline">True</property>
                                    <property name="focus_on_click">False</property>
                                    <property name="draw_indicator">True</property>
                                  </object>
                                  <packing>
                                    <property name="expand">False</property>
                                    <property name="fill">False</property>
                                    <property name="position">0</property>
                                  </packing>
                                </child>
                                <child>
                                  <object class="GtkHBox" id="bc_color_hbox">
                                    <property name="visible">True</property>
                                    <property name="spacing">12</property>
                                    <child>
                                      <placeholder/>
                                    </child>
                                  </object>
                                  <packing>
                                    <property name="position">1</property>
                                  </packing>
                                </child>
                              </object>
                              <packing>
                                <property name="position">0</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkHBox" id="hbox48">
                                <property name="visible">True</property>
                                <property name="spacing">6</property>
                                <child>
                                  <object class="GtkRadioButton" id="bc_key_radio">
                                    <property name="label" translatable="yes">Key:</property>
                                    <property name="visible">True</property>
                                    <property name="can_focus">True</property>
                                    <property name="receives_default">False</property>
                                    <property name="use_underline">True</property>
                                    <property name="focus_on_click">False</property>
                                    <property name="draw_indicator">True</property>
                                    <property name="group">bc_color_radio</property>
                                  </object>
                                  <packing>
                                    <property name="expand">False</property>
                                    <property name="fill">False</property>
                                    <property name="position">0</property>
                                  </packing>
                                </child>
                                <child>
                                  <object class="GtkHBox" id="bc_key_hbox">
                                    <property name="visible">True</property>
                                    <property name="spacing">12</property>
                                    <child>
                                      <placeholder/>
                                    </child>
                                  </object>
                                  <packing>
                                    <property name="position">1</property>
                                  </packing>
                                </child>
                              </object>
                              <packing>
                                <property name="position">1</property>
                              </packing>
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
                        <property name="position">3</property>
                      </packing>
                    </child>
                  </object>
                  <packing>
                    <property name="position">6</property>
                  </packing>
                </child>
                <child type="tab">
                  <object class="GtkLabel" id="bc_tab_label">
                    <property name="visible">True</property>
                    <property name="label" translatable="yes">Style</property>
                  </object>
                  <packing>
                    <property name="position">6</property>
                    <property name="tab_fill">False</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkVBox" id="size_page_vbox">
                    <property name="visible">True</property>
                    <property name="border_width">12</property>
                    <property name="spacing">12</property>
                    <child>
                      <object class="GtkTable" id="table6">
                        <property name="visible">True</property>
                        <property name="n_rows">3</property>
                        <property name="n_columns">3</property>
                        <property name="column_spacing">12</property>
                        <property name="row_spacing">12</property>
                        <child>
                          <object class="GtkLabel" id="size_w_label">
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
                          <object class="GtkLabel" id="size_h_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">Height:</property>
                          </object>
                          <packing>
                            <property name="top_attach">1</property>
                            <property name="bottom_attach">2</property>
                            <property name="x_options">GTK_FILL</property>
                            <property name="y_options"></property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkHBox" id="hbox11">
                            <property name="visible">True</property>
                            <property name="spacing">12</property>
                            <child>
                              <object class="GtkSpinButton" id="size_h_spin">
                                <property name="visible">True</property>
                                <property name="can_focus">True</property>
                                <property name="adjustment">adjustment5</property>
                                <property name="climb_rate">0.0099999997764800008</property>
                                <property name="digits">2</property>
                                <property name="snap_to_ticks">True</property>
                                <property name="numeric">True</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="position">0</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkLabel" id="size_h_units_label">
                                <property name="visible">True</property>
                                <property name="label" translatable="yes">inches</property>
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
                            <property name="top_attach">1</property>
                            <property name="bottom_attach">2</property>
                            <property name="x_options">GTK_FILL</property>
                            <property name="y_options">GTK_FILL</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkHBox" id="hbox13">
                            <property name="visible">True</property>
                            <property name="spacing">12</property>
                            <child>
                              <object class="GtkButton" id="size_reset_image_button">
                                <property name="label" translatable="yes">Reset image size</property>
                                <property name="visible">True</property>
                                <property name="can_focus">True</property>
                                <property name="receives_default">False</property>
                                <property name="use_underline">True</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">False</property>
                                <property name="position">0</property>
                              </packing>
                            </child>
                          </object>
                          <packing>
                            <property name="right_attach">2</property>
                            <property name="top_attach">2</property>
                            <property name="bottom_attach">3</property>
                            <property name="x_options">GTK_FILL</property>
                            <property name="y_options"></property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkHBox" id="hbox10">
                            <property name="visible">True</property>
                            <property name="spacing">12</property>
                            <child>
                              <object class="GtkSpinButton" id="size_w_spin">
                                <property name="visible">True</property>
                                <property name="can_focus">True</property>
                                <property name="adjustment">adjustment4</property>
                                <property name="climb_rate">0.0099999997764800008</property>
                                <property name="digits">2</property>
                                <property name="snap_to_ticks">True</property>
                                <property name="numeric">True</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="position">0</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkLabel" id="size_w_units_label">
                                <property name="visible">True</property>
                                <property name="label" translatable="yes">inches</property>
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
                            <property name="x_options">GTK_FILL</property>
                            <property name="y_options">GTK_FILL</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkVBox" id="size_aspect_vbox">
                            <property name="visible">True</property>
                            <child>
                              <placeholder/>
                            </child>
                          </object>
                          <packing>
                            <property name="left_attach">2</property>
                            <property name="right_attach">3</property>
                            <property name="bottom_attach">2</property>
                            <property name="x_options">GTK_FILL</property>
                            <property name="y_options"></property>
                          </packing>
                        </child>
                        <child>
                          <placeholder/>
                        </child>
                      </object>
                      <packing>
                        <property name="position">0</property>
                      </packing>
                    </child>
                  </object>
                  <packing>
                    <property name="position">7</property>
                  </packing>
                </child>
                <child type="tab">
                  <object class="GtkLabel" id="size_tab_label">
                    <property name="visible">True</property>
                    <property name="label" translatable="yes">Size</property>
                  </object>
                  <packing>
                    <property name="position">7</property>
                    <property name="tab_fill">False</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkVBox" id="lsize_page_vbox">
                    <property name="visible">True</property>
                    <property name="border_width">12</property>
                    <property name="orientation">vertical</property>
                    <property name="spacing">12</property>
                    <child>
                      <object class="GtkTable" id="table7">
                        <property name="visible">True</property>
                        <property name="n_rows">2</property>
                        <property name="n_columns">2</property>
                        <property name="column_spacing">12</property>
                        <property name="row_spacing">12</property>
                        <child>
                          <object class="GtkLabel" id="lsize_r_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">Length:</property>
                          </object>
                          <packing>
                            <property name="x_options">GTK_FILL</property>
                            <property name="y_options"></property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkLabel" id="lsize_theta_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">Angle:</property>
                          </object>
                          <packing>
                            <property name="top_attach">1</property>
                            <property name="bottom_attach">2</property>
                            <property name="x_options">GTK_FILL</property>
                            <property name="y_options"></property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkHBox" id="hbox25">
                            <property name="visible">True</property>
                            <property name="spacing">12</property>
                            <child>
                              <object class="GtkSpinButton" id="lsize_r_spin">
                                <property name="visible">True</property>
                                <property name="can_focus">True</property>
                                <property name="adjustment">adjustment6</property>
                                <property name="climb_rate">0.0099999997764800008</property>
                                <property name="digits">2</property>
                                <property name="snap_to_ticks">True</property>
                                <property name="numeric">True</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="position">0</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkLabel" id="lsize_r_units_label">
                                <property name="visible">True</property>
                                <property name="label" translatable="yes">inches</property>
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
                          <object class="GtkHBox" id="hbox26">
                            <property name="visible">True</property>
                            <property name="spacing">12</property>
                            <child>
                              <object class="GtkSpinButton" id="lsize_theta_spin">
                                <property name="visible">True</property>
                                <property name="can_focus">True</property>
                                <property name="adjustment">adjustment7</property>
                                <property name="climb_rate">0.0099999997764800008</property>
                                <property name="digits">1</property>
                                <property name="snap_to_ticks">True</property>
                                <property name="numeric">True</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="position">0</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkLabel" id="label38">
                                <property name="visible">True</property>
                                <property name="label" translatable="yes">degrees</property>
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
                            <property name="top_attach">1</property>
                            <property name="bottom_attach">2</property>
                            <property name="x_options">GTK_FILL</property>
                            <property name="y_options">GTK_FILL</property>
                          </packing>
                        </child>
                      </object>
                      <packing>
                        <property name="position">0</property>
                      </packing>
                    </child>
                  </object>
                  <packing>
                    <property name="position">8</property>
                  </packing>
                </child>
                <child type="tab">
                  <object class="GtkLabel" id="lsize_tab_label">
                    <property name="visible">True</property>
                    <property name="label" translatable="yes">Size</property>
                  </object>
                  <packing>
                    <property name="position">8</property>
                    <property name="tab_fill">False</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkVBox" id="pos_page_vbox">
                    <property name="visible">True</property>
                    <property name="border_width">12</property>
                    <property name="spacing">12</property>
                    <child>
                      <object class="GtkTable" id="table8">
                        <property name="visible">True</property>
                        <property name="n_rows">2</property>
                        <property name="n_columns">3</property>
                        <property name="column_spacing">12</property>
                        <property name="row_spacing">12</property>
                        <child>
                          <object class="GtkLabel" id="pos_x_label">
                            <property name="width_request">50</property>
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">X:</property>
                          </object>
                          <packing>
                            <property name="x_options">GTK_FILL</property>
                            <property name="y_options"></property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkLabel" id="pos_y_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">Y:</property>
                          </object>
                          <packing>
                            <property name="top_attach">1</property>
                            <property name="bottom_attach">2</property>
                            <property name="x_options">GTK_FILL</property>
                            <property name="y_options"></property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkLabel" id="pos_x_units_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">inches</property>
                          </object>
                          <packing>
                            <property name="left_attach">2</property>
                            <property name="right_attach">3</property>
                            <property name="x_options">GTK_FILL</property>
                            <property name="y_options"></property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkLabel" id="pos_y_units_label">
                            <property name="visible">True</property>
                            <property name="xalign">0</property>
                            <property name="label" translatable="yes">inches</property>
                          </object>
                          <packing>
                            <property name="left_attach">2</property>
                            <property name="right_attach">3</property>
                            <property name="top_attach">1</property>
                            <property name="bottom_attach">2</property>
                            <property name="x_options">GTK_FILL</property>
                            <property name="y_options"></property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkSpinButton" id="pos_x_spin">
                            <property name="visible">True</property>
                            <property name="can_focus">True</property>
                            <property name="adjustment">adjustment8</property>
                            <property name="climb_rate">0.0099999997764800008</property>
                            <property name="digits">2</property>
                            <property name="snap_to_ticks">True</property>
                            <property name="numeric">True</property>
                          </object>
                          <packing>
                            <property name="left_attach">1</property>
                            <property name="right_attach">2</property>
                            <property name="x_options"></property>
                            <property name="y_options"></property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkSpinButton" id="pos_y_spin">
                            <property name="visible">True</property>
                            <property name="can_focus">True</property>
                            <property name="adjustment">adjustment9</property>
                            <property name="climb_rate">0.0099999997764800008</property>
                            <property name="digits">2</property>
                            <property name="snap_to_ticks">True</property>
                            <property name="numeric">True</property>
                          </object>
                          <packing>
                            <property name="left_attach">1</property>
                            <property name="right_attach">2</property>
                            <property name="top_attach">1</property>
                            <property name="bottom_attach">2</property>
                            <property name="x_options"></property>
                            <property name="y_options"></property>
                          </packing>
                        </child>
                      </object>
                      <packing>
                        <property name="position">0</property>
                      </packing>
                    </child>
                  </object>
                  <packing>
                    <property name="position">9</property>
                  </packing>
                </child>
                <child type="tab">
                  <object class="GtkLabel" id="pos_tab_label">
                    <property name="visible">True</property>
                    <property name="label" translatable="yes">Position</property>
                  </object>
                  <packing>
                    <property name="position">9</property>
                    <property name="tab_fill">False</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkVBox" id="shadow_page_vbox">
                    <property name="visible">True</property>
                    <property name="border_width">12</property>
                    <property name="orientation">vertical</property>
                    <property name="spacing">12</property>
                    <child>
                      <object class="GtkCheckButton" id="shadow_enable_check">
                        <property name="label" translatable="yes">Enable shadow</property>
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
                      <object class="GtkVBox" id="shadow_controls_table">
                        <property name="visible">True</property>
                        <property name="orientation">vertical</property>
                        <property name="spacing">12</property>
                        <child>
                          <object class="GtkHBox" id="hbox63">
                            <property name="visible">True</property>
                            <property name="spacing">12</property>
                            <child>
                              <object class="GtkLabel" id="label40">
                                <property name="visible">True</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">X Offset:</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">False</property>
                                <property name="position">0</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkHBox" id="hbox54">
                                <property name="visible">True</property>
                                <property name="spacing">12</property>
                                <child>
                                  <object class="GtkSpinButton" id="shadow_x_spin">
                                    <property name="visible">True</property>
                                    <property name="can_focus">True</property>
                                    <property name="adjustment">adjustment10</property>
                                    <property name="climb_rate">0.0099999997764800008</property>
                                    <property name="digits">2</property>
                                    <property name="snap_to_ticks">True</property>
                                    <property name="numeric">True</property>
                                  </object>
                                  <packing>
                                    <property name="expand">False</property>
                                    <property name="position">0</property>
                                  </packing>
                                </child>
                                <child>
                                  <object class="GtkLabel" id="shadow_x_units_label">
                                    <property name="visible">True</property>
                                    <property name="label" translatable="yes">inches</property>
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
                          <packing>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkHBox" id="hbox64">
                            <property name="visible">True</property>
                            <property name="spacing">12</property>
                            <child>
                              <object class="GtkLabel" id="label41">
                                <property name="visible">True</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">Y Offset:</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">False</property>
                                <property name="position">0</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkHBox" id="hbox55">
                                <property name="visible">True</property>
                                <property name="spacing">12</property>
                                <child>
                                  <object class="GtkSpinButton" id="shadow_y_spin">
                                    <property name="visible">True</property>
                                    <property name="can_focus">True</property>
                                    <property name="adjustment">adjustment11</property>
                                    <property name="climb_rate">0.0099999997764800008</property>
                                    <property name="digits">2</property>
                                    <property name="snap_to_ticks">True</property>
                                    <property name="numeric">True</property>
                                  </object>
                                  <packing>
                                    <property name="expand">False</property>
                                    <property name="position">0</property>
                                  </packing>
                                </child>
                                <child>
                                  <object class="GtkLabel" id="shadow_y_units_label">
                                    <property name="visible">True</property>
                                    <property name="label" translatable="yes">inches</property>
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
                          <packing>
                            <property name="position">1</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkHBox" id="hbox61">
                            <property name="visible">True</property>
                            <property name="spacing">12</property>
                            <child>
                              <object class="GtkLabel" id="label45">
                                <property name="visible">True</property>
                                <property name="xalign">0</property>
                                <property name="yalign">0.14000000059604645</property>
                                <property name="label" translatable="yes">Color:</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">False</property>
                                <property name="position">0</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkVBox" id="vbox7">
                                <property name="visible">True</property>
                                <property name="orientation">vertical</property>
                                <property name="spacing">6</property>
                                <child>
                                  <object class="GtkHBox" id="hbox57">
                                    <property name="visible">True</property>
                                    <property name="spacing">6</property>
                                    <child>
                                      <object class="GtkRadioButton" id="shadow_color_radio">
                                        <property name="visible">True</property>
                                        <property name="can_focus">True</property>
                                        <property name="receives_default">False</property>
                                        <property name="use_underline">True</property>
                                        <property name="focus_on_click">False</property>
                                        <property name="draw_indicator">True</property>
                                      </object>
                                      <packing>
                                        <property name="expand">False</property>
                                        <property name="fill">False</property>
                                        <property name="position">0</property>
                                      </packing>
                                    </child>
                                    <child>
                                      <object class="GtkHBox" id="shadow_color_hbox">
                                        <property name="visible">True</property>
                                        <property name="spacing">12</property>
                                        <child>
                                          <placeholder/>
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
                                  <object class="GtkHBox" id="hbox59">
                                    <property name="visible">True</property>
                                    <property name="spacing">6</property>
                                    <child>
                                      <object class="GtkRadioButton" id="shadow_key_radio">
                                        <property name="label" translatable="yes">Key:</property>
                                        <property name="visible">True</property>
                                        <property name="can_focus">True</property>
                                        <property name="receives_default">False</property>
                                        <property name="use_underline">True</property>
                                        <property name="focus_on_click">False</property>
                                        <property name="draw_indicator">True</property>
                                        <property name="group">shadow_color_radio</property>
                                      </object>
                                      <packing>
                                        <property name="expand">False</property>
                                        <property name="fill">False</property>
                                        <property name="position">0</property>
                                      </packing>
                                    </child>
                                    <child>
                                      <object class="GtkHBox" id="shadow_key_hbox">
                                        <property name="visible">True</property>
                                        <property name="spacing">12</property>
                                        <child>
                                          <placeholder/>
                                        </child>
                                      </object>
                                      <packing>
                                        <property name="position">1</property>
                                      </packing>
                                    </child>
                                  </object>
                                  <packing>
                                    <property name="position">1</property>
                                  </packing>
                                </child>
                              </object>
                              <packing>
                                <property name="position">1</property>
                              </packing>
                            </child>
                          </object>
                          <packing>
                            <property name="position">2</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkHBox" id="hbox62">
                            <property name="visible">True</property>
                            <property name="spacing">12</property>
                            <child>
                              <object class="GtkLabel" id="label46">
                                <property name="visible">True</property>
                                <property name="xalign">0</property>
                                <property name="label" translatable="yes">Opacity:</property>
                              </object>
                              <packing>
                                <property name="expand">False</property>
                                <property name="fill">False</property>
                                <property name="position">0</property>
                              </packing>
                            </child>
                            <child>
                              <object class="GtkHBox" id="hbox56">
                                <property name="visible">True</property>
                                <property name="spacing">12</property>
                                <child>
                                  <object class="GtkSpinButton" id="shadow_opacity_spin">
                                    <property name="visible">True</property>
                                    <property name="can_focus">True</property>
                                    <property name="adjustment">adjustment12</property>
                                    <property name="climb_rate">1</property>
                                  </object>
                                  <packing>
                                    <property name="expand">False</property>
                                    <property name="position">0</property>
                                  </packing>
                                </child>
                                <child>
                                  <object class="GtkLabel" id="label47">
                                    <property name="visible">True</property>
                                    <property name="label" translatable="yes">%</property>
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
                          <packing>
                            <property name="position">3</property>
                          </packing>
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
                    <property name="position">10</property>
                  </packing>
                </child>
                <child type="tab">
                  <object class="GtkLabel" id="shadow_tab_label">
                    <property name="visible">True</property>
                    <property name="label" translatable="yes">Shadow</property>
                  </object>
                  <packing>
                    <property name="position">10</property>
                    <property name="tab_fill">False</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkVBox" id="dummy_vbox">
                    <property name="visible">True</property>
                    <child>
                      <placeholder/>
                    </child>
                  </object>
                  <packing>
                    <property name="position">11</property>
                  </packing>
                </child>
                <child type="tab">
                  <object class="GtkLabel" id="dummy_page">
                    <property name="visible">True</property>
                    <property name="label" translatable="yes">label48</property>
                  </object>
                  <packing>
                    <property name="position">11</property>
                    <property name="tab_fill">False</property>
                  </packing>
                </child>
              </object>
              <packing>
                <property name="position">1</property>
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
  <object class="GtkAdjustment" id="adjustment4">
    <property name="value">1</property>
    <property name="upper">100</property>
    <property name="step_increment">0.01</property>
    <property name="page_increment">0.10000000000000001</property>
  </object>
  <object class="GtkAdjustment" id="adjustment5">
    <property name="value">1</property>
    <property name="upper">100</property>
    <property name="step_increment">0.01</property>
    <property name="page_increment">0.10000000000000001</property>
  </object>
  <object class="GtkAdjustment" id="adjustment6">
    <property name="value">1</property>
    <property name="upper">100</property>
    <property name="step_increment">0.01</property>
    <property name="page_increment">0.10000000000000001</property>
  </object>
  <object class="GtkAdjustment" id="adjustment7">
    <property name="lower">-180</property>
    <property name="upper">180</property>
    <property name="step_increment">0.10000000000000001</property>
    <property name="page_increment">5</property>
  </object>
  <object class="GtkAdjustment" id="adjustment8">
    <property name="upper">100</property>
    <property name="step_increment">0.01</property>
    <property name="page_increment">1</property>
  </object>
  <object class="GtkAdjustment" id="adjustment9">
    <property name="upper">100</property>
    <property name="step_increment">0.01</property>
    <property name="page_increment">1</property>
  </object>
  <object class="GtkAdjustment" id="adjustment10">
    <property name="upper">100</property>
    <property name="step_increment">0.01</property>
    <property name="page_increment">1</property>
  </object>
  <object class="GtkAdjustment" id="adjustment11">
    <property name="upper">100</property>
    <property name="step_increment">0.01</property>
    <property name="page_increment">1</property>
  </object>
  <object class="GtkAdjustment" id="adjustment12">
    <property name="value">1</property>
    <property name="upper">100</property>
    <property name="step_increment">1</property>
    <property name="page_increment">10</property>
  </object>
  <object class="GtkAdjustment" id="adjustment13">
    <property name="value">1</property>
    <property name="lower">1</property>
    <property name="upper">100</property>
    <property name="step_increment">1</property>
    <property name="page_increment">10</property>
  </object>
</interface>
