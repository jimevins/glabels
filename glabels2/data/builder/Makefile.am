## Process this file with automake to produce Makefile.in

builderdir = $(datadir)/glabels/builder/

builder_DATA = \
	property-bar.builder				\
	print-custom-widget.builder			\
	wdgt-media-select.builder			\
	merge-properties-dialog.builder			\
	new-label-dialog.builder			\
	template-designer-start-page.builder		\
	template-designer-name-page.builder		\
	template-designer-pg-size-page.builder		\
	template-designer-shape-page.builder		\
	template-designer-rect-size-page.builder	\
	template-designer-round-size-page.builder	\
	template-designer-cd-size-page.builder		\
	template-designer-nlayouts-page.builder		\
	template-designer-layout-page.builder		\
	template-designer-finish-page.builder		\
	prefs-dialog.builder				\
	object-editor.builder


EXTRA_DIST = \
	object-editor.glade		\
	prefs-dialog.glade		\
	template-designer.glade		\
	merge-properties-dialog.glade	\
	property-bar.glade		\
	print-custom-widget.glade	\
	wdgt-media-select.glade		\
	new-label-dialog.glade

CLEANFILES = $(builder_DATA)


property-bar.builder: property-bar.glade
	$(GTK_BUILDER_CONVERT) --root property_toolbar $< $@

print-custom-widget.builder: print-custom-widget.glade
	$(GTK_BUILDER_CONVERT) --root print_custom_widget_vbox $< $@

wdgt-media-select.builder: wdgt-media-select.glade
	$(GTK_BUILDER_CONVERT) --root wdgt_media_select_hbox $< $@

merge-properties-dialog.builder: merge-properties-dialog.glade
	$(GTK_BUILDER_CONVERT) --root merge_properties_vbox $< $@

new-label-dialog.builder: new-label-dialog.glade
	$(GTK_BUILDER_CONVERT) --root new_label_dialog_vbox $< $@

prefs-dialog.builder: prefs-dialog.glade
	$(GTK_BUILDER_CONVERT) --root prefs_notebook $< $@

object-editor.builder: object-editor.glade
	$(GTK_BUILDER_CONVERT) --root editor_vbox $< $@

template-designer-start-page.builder: template-designer.glade
	$(GTK_BUILDER_CONVERT) --root start_page $< $@
template-designer-name-page.builder: template-designer.glade
	$(GTK_BUILDER_CONVERT) --root name_page $< $@
template-designer-pg-size-page.builder: template-designer.glade
	$(GTK_BUILDER_CONVERT) --root pg_size_page $< $@
template-designer-shape-page.builder: template-designer.glade
	$(GTK_BUILDER_CONVERT) --root shape_page $< $@
template-designer-rect-size-page.builder: template-designer.glade
	$(GTK_BUILDER_CONVERT) --root rect_size_page $< $@
template-designer-round-size-page.builder: template-designer.glade
	$(GTK_BUILDER_CONVERT) --root round_size_page $< $@
template-designer-cd-size-page.builder: template-designer.glade
	$(GTK_BUILDER_CONVERT) --root cd_size_page $< $@
template-designer-nlayouts-page.builder: template-designer.glade
	$(GTK_BUILDER_CONVERT) --root nlayouts_page $< $@
template-designer-layout-page.builder: template-designer.glade
	$(GTK_BUILDER_CONVERT) --root layout_page $< $@
template-designer-finish-page.builder: template-designer.glade
	$(GTK_BUILDER_CONVERT) --root finish_page $< $@

