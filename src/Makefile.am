## Process this file with automake to produce Makefile.in

INCLUDES = \
	@GTK_CFLAGS@ \
	$(GNOME_INCLUDEDIR)

pkgdata_DATA =	vide0.5.2.txt

bin_PROGRAMS = vide 

bin_SCRIPTS = pauseme

vide_SOURCES = \
	vide.c vide.h \
	add_ext_dialog.c \
	bookmarks.c \
	buffer_menu.c \
	callbacks.c \
	command_mode.c \
	command_mode_menu.c \
	config_dialog.c \
	config_files.c \
	confirm_dialog.c \
	copy_here_dialog.c \
	date_filter_dialog.c \
	file_info_dialog.c \
	filelist.c \
	filename_filter_dialog.c \
	fileops.c \
	fileselector.c \
	filetype.c \
	filetype_dialog.c \
	filetype_popup.c \
	for_each_dialog.c \
	history.c \
	init_filetype_dialog.c \
	main_menu.c \
	misc_dialogs.c \
	ownership_dialog.c \
	permissions_dialog.c \
	programs_dialog.c \
	programs_menu.c \
	rename_ext_dialog.c \
	search_mode.c \
	size_filter_dialog.c \
	term.c \
	term_menu.c \
	user_command.c \
	user_command_dialog.c \
	user_prompt.c \
	utils.c \
	view_dialog.c \
	widget_utils.c \
	window.c 

vide_LDADD= $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(ZVT_LIBS) @GTK_LIBS@  
