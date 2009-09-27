<?xml version="1.0"?>
<interface>
  <requires lib="gtk+" version="2.16"/>
  <!-- interface-naming-policy toplevel-contextual -->
  <object class="GtkWindow" id="window1">
    <property name="visible">True</property>
    <property name="title" translatable="yes">window1</property>
    <child>
      <object class="GtkHBox" id="wdgt_media_select_hbox">
        <property name="visible">True</property>
        <child>
          <object class="GtkNotebook" id="notebook">
            <property name="visible">True</property>
            <property name="border_width">12</property>
            <child>
              <object class="GtkVBox" id="recent_tab_vbox">
                <property name="visible">True</property>
                <child>
                  <object class="GtkScrolledWindow" id="scrolledwindow2">
                    <property name="width_request">480</property>
                    <property name="height_request">320</property>
                    <property name="visible">True</property>
                    <property name="can_focus">True</property>
                    <property name="border_width">12</property>
                    <property name="hscrollbar_policy">automatic</property>
                    <property name="vscrollbar_policy">automatic</property>
                    <property name="shadow_type">in</property>
                    <child>
                      <object class="GtkTreeView" id="recent_treeview">
                        <property name="visible">True</property>
                        <property name="headers_visible">False</property>
                      </object>
                    </child>
                  </object>
                  <packing>
                    <property name="position">0</property>
                  </packing>
                </child>
              </object>
            </child>
            <child type="tab">
              <object class="GtkLabel" id="recent_tab_label">
                <property name="visible">True</property>
                <property name="label" translatable="yes">Recent templates</property>
              </object>
              <packing>
                <property name="tab_fill">False</property>
              </packing>
            </child>
            <child>
              <object class="GtkVBox" id="search_all_tab_vbox">
                <property name="visible">True</property>
                <child>
                  <object class="GtkHBox" id="hbox6">
                    <property name="visible">True</property>
                    <property name="border_width">12</property>
                    <property name="spacing">12</property>
                    <child>
                      <object class="GtkHBox" id="hbox9">
                        <property name="visible">True</property>
                        <property name="spacing">6</property>
                        <child>
                          <object class="GtkLabel" id="label14">
                            <property name="visible">True</property>
                            <property name="label" translatable="yes">Brand:</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkComboBox" id="brand_combo">
                            <property name="visible">True</property>
                            <property name="focus_on_click">False</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
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
                      <object class="GtkHBox" id="hbox7">
                        <property name="visible">True</property>
                        <property name="spacing">6</property>
                        <child>
                          <object class="GtkLabel" id="label12">
                            <property name="visible">True</property>
                            <property name="label" translatable="yes">Page size:</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkComboBox" id="page_size_combo">
                            <property name="visible">True</property>
                            <property name="focus_on_click">False</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="position">1</property>
                          </packing>
                        </child>
                      </object>
                      <packing>
                        <property name="expand">False</property>
                        <property name="position">1</property>
                      </packing>
                    </child>
                    <child>
                      <object class="GtkHBox" id="hbox8">
                        <property name="visible">True</property>
                        <property name="spacing">6</property>
                        <child>
                          <object class="GtkLabel" id="label13">
                            <property name="visible">True</property>
                            <property name="label" translatable="yes">Category:</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="fill">False</property>
                            <property name="position">0</property>
                          </packing>
                        </child>
                        <child>
                          <object class="GtkComboBox" id="category_combo">
                            <property name="visible">True</property>
                            <property name="focus_on_click">False</property>
                          </object>
                          <packing>
                            <property name="expand">False</property>
                            <property name="position">1</property>
                          </packing>
                        </child>
                      </object>
                      <packing>
                        <property name="expand">False</property>
                        <property name="position">2</property>
                      </packing>
                    </child>
                  </object>
                  <packing>
                    <property name="expand">False</property>
                    <property name="position">0</property>
                  </packing>
                </child>
                <child>
                  <object class="GtkScrolledWindow" id="scrolledwindow1">
                    <property name="width_request">480</property>
                    <property name="height_request">320</property>
                    <property name="visible">True</property>
                    <property name="can_focus">True</property>
                    <property name="border_width">12</property>
                    <property name="hscrollbar_policy">automatic</property>
                    <property name="vscrollbar_policy">automatic</property>
                    <property name="shadow_type">in</property>
                    <child>
                      <object class="GtkTreeView" id="search_all_treeview">
                        <property name="visible">True</property>
                        <property name="headers_visible">False</property>
                      </object>
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
            <child type="tab">
              <object class="GtkLabel" id="search_all_tabLlabel">
                <property name="visible">True</property>
                <property name="label" translatable="yes">Search all templates</property>
              </object>
              <packing>
                <property name="position">1</property>
                <property name="tab_fill">False</property>
              </packing>
            </child>
          </object>
          <packing>
            <property name="position">0</property>
          </packing>
        </child>
      </object>
    </child>
  </object>
</interface>
