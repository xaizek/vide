/* vide
 * Copyright (C) 2000 Ken Steen.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vide.h"

static GtkWidget *dialog;
static GtkWidget *colorsel_dialog;

static struct
{
  GtkWidget *right_startup_dir_entry;
  GtkWidget *left_startup_dir_entry;
  GtkWidget *start_with_cwd_checkbutton;
	GtkWidget *vim;
	GtkWidget *vile;
	GtkWidget *elvis;
	GtkWidget *nvi;
	GtkWidget *vi_entry;
}
general_page_1;

static struct
{
  GtkWidget *dir_history_entry;
  GtkWidget *command_history_entry;
  GtkWidget *use_scrollbars;
	GtkWidget *save_position;
  GtkWidget *xterm_entry;
	GtkWidget *filter_entry;
	GtkWidget *invert_filter_check_button;
}
general_page_2;

static struct
{
  GtkWidget *clist;
  gint row;
}
bookmarks_page;

static struct
{
  GtkWidget *clist;
  GtkWidget *default_program_label;
}
filetypes_page;

static struct
{
  GtkWidget *clist;
}
user_commands_page;

static struct
{
	GtkWidget *clist;
}
programs_page;

static struct
{
  GtkWidget *checks[MAX_COLUMNS];
}
columns_page;

static struct
{
	GtkWidget *blinking_cursor;
	GtkWidget *term_bell;
	GtkWidget *scroll_key;
	GtkWidget *scroll_output;
	GtkWidget *swap;
	GtkWidget *transparent;
/*	GtkWidget *shaded;
	GtkWidget *pixmap;
	GtkWidget *pixmap_entry;
	*/
	GtkWidget *scroll_menu;
	GtkWidget *tab_menu;
	GtkWidget *term_font;
	GtkWidget *scheme_menu;
	GtkWidget *background_menu;
	GtkWidget *tab_button;
	GtkObject *adj;
	GtkWidget *spin;
	GtkWidget *entry;
	GtkWidget *popdown;
	GtkWidget *color_button[20];
	GtkWidget *program_entry;
}zvt_general;

static struct
{
	gchar color_scheme[512];
	gchar background[PATH_MAX];
	GtkPositionType tab_position;
	gchar font[512];
	gchar pixmap[512];
	gboolean show_tabs;
	gint color_change;
	gint background_change;
	gint tab_change;
	gint pixmap_change;
	gint font_change;
	gint hide_change;
}
tmp_values;

enum { LINUX, XTERM, RXVT, CUSTOM,
			 WHITE_ON_BLACK, BLACK_ON_WHITE, GREEN_ON_BLACK, CUSTOM_BACK,
			 HIDDEN, LEFT, RIGHT, TOP, BOTTOM };
void
config_key_press_cb (GtkWidget * widget, GdkEventKey * event, gpointer data)
{
  if ((event->keyval < 0x100)
      && (event->state == 0 || event->state & GDK_SHIFT_MASK
	  || event->state & GDK_MOD2_MASK))
    {
      switch (event->keyval)
	{

	case 'j':
	  event->keyval = GDK_Down;
	  break;
	case 'k':
	  event->keyval = GDK_Up;
	  break;

	case 'h':
	  event->keyval = GDK_Right;

	case 'l':
	  event->keyval = GDK_Left;
	  break;

	default:
	  event->keyval = 0;
	  break;
	}
    }
  return;
}

static void
ok_cb (GtkWidget * button)
{
  gint i;

	zvt.scroll_lines = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(
				zvt_general.spin));
	if(GTK_TOGGLE_BUTTON(general_page_1.vim)->active)
		g_snprintf(cfg.vi_clone, sizeof(cfg.vi_clone), "vim");
	if(GTK_TOGGLE_BUTTON(general_page_1.vile)->active)
		g_snprintf(cfg.vi_clone, sizeof(cfg.vi_clone), "vile");
	if(GTK_TOGGLE_BUTTON(general_page_1.elvis)->active)
		g_snprintf(cfg.vi_clone, sizeof(cfg.vi_clone), "elvis");
	if(GTK_TOGGLE_BUTTON(general_page_1.nvi)->active)
		g_snprintf(cfg.vi_clone, sizeof(cfg.vi_clone), "vi");
	strncpy(cfg.vi_command, 
			gtk_entry_get_text(GTK_ENTRY(general_page_1.vi_entry)),
			sizeof(cfg.vi_command));

	strncpy(cfg.man_program, 
			gtk_entry_get_text(GTK_ENTRY(zvt_general.program_entry)),
			sizeof(cfg.man_program));
	zvt.use_blinking_cursor=
		GTK_TOGGLE_BUTTON(zvt_general.blinking_cursor)->active;
	zvt.terminal_bell=
		GTK_TOGGLE_BUTTON(zvt_general.term_bell)->active;
	zvt.scroll_on_keystroke=
		GTK_TOGGLE_BUTTON(zvt_general.scroll_key)->active;
	zvt.scroll_on_output=
		GTK_TOGGLE_BUTTON(zvt_general.scroll_output)->active;
	zvt.swap_backspace=
		GTK_TOGGLE_BUTTON(zvt_general.swap)->active;
	zvt.transparent=
		GTK_TOGGLE_BUTTON(zvt_general.transparent)->active;
	if(tmp_values.hide_change || tmp_values.tab_change)
		zvt.show_tabs = tmp_values.show_tabs;
	if(tmp_values.tab_change)
	zvt.tab_position = (GtkPositionType)tmp_values.tab_position;
	if(tmp_values.color_change)
	g_snprintf(zvt.term_color, sizeof(zvt.term_color), tmp_values.color_scheme);
	if(tmp_values.background_change)
	g_snprintf(zvt.background, sizeof(zvt.background), tmp_values.background);
	if(tmp_values.font_change)
	g_snprintf(zvt.term_font, sizeof(zvt.term_font), tmp_values.font);
	if(tmp_values.pixmap_change)
	g_snprintf(zvt.pixmap, sizeof(zvt.pixmap), tmp_values.pixmap);

	cfg.invert_filter=
		GTK_TOGGLE_BUTTON(general_page_2.invert_filter_check_button)->active;
	strncpy(cfg.start_filter, 
			gtk_entry_get_text(GTK_ENTRY(general_page_2.filter_entry)),
			sizeof(cfg.start_filter));
	cfg.use_scrollbars =
		GTK_TOGGLE_BUTTON(general_page_2.use_scrollbars)->active;
	cfg.save_position =
		GTK_TOGGLE_BUTTON(general_page_2.save_position)->active;
	cfg.start_with_cwd =
		GTK_TOGGLE_BUTTON(general_page_1.start_with_cwd_checkbutton)->active;
  strncpy (cfg.left_startup_dir,
	   gtk_entry_get_text (GTK_ENTRY
			       (general_page_1.left_startup_dir_entry)),
	   sizeof (cfg.left_startup_dir));
  strncpy (cfg.right_startup_dir,
	   gtk_entry_get_text (GTK_ENTRY
			       (general_page_1.right_startup_dir_entry)),
	   sizeof (cfg.right_startup_dir));
  strncpy (cfg.xterm_command,
	   gtk_entry_get_text (GTK_ENTRY (general_page_2.xterm_entry)),
	   sizeof (cfg.xterm_command));
  cfg.dir_history_max_length =
    atoi (gtk_entry_get_text (GTK_ENTRY (general_page_2.dir_history_entry)));
  cfg.command_history_max_length =
    atoi (gtk_entry_get_text
	  (GTK_ENTRY (general_page_2.command_history_entry)));

	g_list_free(cfg.bookmarks);
  cfg.bookmarks = NULL;
  for (i = 0; i < GTK_CLIST (bookmarks_page.clist)->rows; i++)
    {
			Bookmarks *bk;
			bk = gtk_clist_get_row_data(GTK_CLIST(bookmarks_page.clist), i);
      cfg.bookmarks = g_list_append (cfg.bookmarks, bk);
    }

	g_list_free(cfg.filetypes);
  cfg.filetypes = NULL;
  for (i = 0; i < GTK_CLIST (filetypes_page.clist)->rows; i++)
    {
      FileType *ft;
      ft = gtk_clist_get_row_data (GTK_CLIST (filetypes_page.clist), i);
      cfg.filetypes = g_list_append (cfg.filetypes, ft);
    }

	g_list_free(cfg.user_commands);
  cfg.user_commands = NULL;
  for (i = 0; i < GTK_CLIST (user_commands_page.clist)->rows; i++)
    {
      UserCommand *command;
      command =
	gtk_clist_get_row_data (GTK_CLIST (user_commands_page.clist), i);
      cfg.user_commands = g_list_append (cfg.user_commands, command);
    }

  for (i = 0; i < MAX_COLUMNS; i++)
    {
      all_columns[i].is_visible =
	GTK_TOGGLE_BUTTON (columns_page.checks[i])->active;
    }

  /* the dialog needs to be destroyed before we refresh the main window */
  gtk_grab_remove (dialog);
  gtk_widget_destroy (dialog);
	if( 1 < gtk_main_level())
		gtk_main_quit();

  recreate_main_window ();

  write_filetypes_file ();
  write_bookmarks_file ();
  write_user_commands_file ();
  write_config_file ();
	write_term_file();
}

static void
cancel_cb (GtkWidget * button)
{
  gtk_grab_remove (dialog);
  gtk_widget_destroy (dialog);
	if(1 < gtk_main_level())
		gtk_main_quit();
}

static void
key_press_cb (GtkWidget * widget, GdkEventKey * event, gpointer data)
{
  if (event->keyval == GDK_Escape)
    cancel_cb (NULL);
}
/*
 * General Page Callbacks
 */
static void
start_with_cwd_cb(GtkWidget *check_button)
{
	gtk_widget_set_sensitive(general_page_1.right_startup_dir_entry,
			!GTK_TOGGLE_BUTTON(check_button)->active);
		gtk_widget_set_sensitive(general_page_1.left_startup_dir_entry,
			!GTK_TOGGLE_BUTTON(check_button)->active);
}

/*
 * Bookmark Page Callbacks
 */
static void
bookmarks_select_row_cb (GtkWidget * clist, gint row, gint column,
			 GdkEvent * event)
{
  bookmarks_page.row = row;
}

static void
bookmark_up_cb (GtkWidget * button, GtkWidget * clist)
{
  if (bookmarks_page.row < 1)
    return;

  gtk_clist_swap_rows (GTK_CLIST (clist), bookmarks_page.row,
		       bookmarks_page.row - 1);
  gtk_clist_select_row (GTK_CLIST (clist), bookmarks_page.row - 1, 0);
}

static void
bookmark_down_cb (GtkWidget * button, GtkWidget * clist)
{
  if (bookmarks_page.row >= (GTK_CLIST (clist)->rows - 1))
    return;

  gtk_clist_swap_rows (GTK_CLIST (clist), bookmarks_page.row,
		       bookmarks_page.row + 1);
  gtk_clist_select_row (GTK_CLIST (clist), bookmarks_page.row + 1, 0);
}

static void
bookmark_remove_cb (GtkWidget * button, GtkWidget * clist)
{
  gtk_clist_remove (GTK_CLIST (clist), bookmarks_page.row);
}

/*
 * Color Page Callbacks
 */
static void
color_select_ok_cb (GtkWidget * button, GdkColor * color)
{
  gdouble rgb[3];

  gtk_color_selection_get_color (GTK_COLOR_SELECTION
				 (GTK_COLOR_SELECTION_DIALOG
				  (colorsel_dialog)->colorsel), rgb);

  color->red = rgb[0] * 65535.0;
  color->green = rgb[1] * 65535.0;
  color->blue = rgb[2] * 65535.0;

  gtk_grab_remove (colorsel_dialog);
  gtk_widget_destroy (colorsel_dialog);
	if(1 < gtk_main_level())
		gtk_main_quit();
}

static void
color_select_cancel_cb (GtkWidget * button)
{
  gtk_grab_remove (colorsel_dialog);
  gtk_widget_destroy (colorsel_dialog);
	if( 1 < gtk_main_level())
		gtk_main_quit();
}

static void
color_changed_cb (GtkWidget * widget, GtkWidget * button)
{
  GdkColor color;
  gdouble rgb[3];
  GtkStyle *style = gtk_style_new ();

  gtk_color_selection_get_color (GTK_COLOR_SELECTION (widget), rgb);

  color.red = rgb[0] * 65535.0;
  color.green = rgb[1] * 65535.0;
  color.blue = rgb[2] * 65535.0;

  style->bg[GTK_STATE_NORMAL] = color;
  gtk_widget_set_style (button, style);
}

static void
color_select_cb (GtkWidget * button, GdkColor * color)
{
  gdouble rgb[3];

  colorsel_dialog = gtk_color_selection_dialog_new ("Choose Color");
  gtk_widget_destroy (GTK_COLOR_SELECTION_DIALOG (colorsel_dialog)->
		      help_button);

  rgb[0] = color->red / 65535.0;
  rgb[1] = color->green / 65535.0;
  rgb[2] = color->blue / 65535.0;
  gtk_color_selection_set_color (GTK_COLOR_SELECTION
				 (GTK_COLOR_SELECTION_DIALOG
				  (colorsel_dialog)->colorsel), rgb);

  gtk_signal_connect (GTK_OBJECT
		      (GTK_COLOR_SELECTION_DIALOG (colorsel_dialog)->
		       ok_button), "clicked",
		      GTK_SIGNAL_FUNC (color_select_ok_cb), color);
  gtk_signal_connect (GTK_OBJECT
		      (GTK_COLOR_SELECTION_DIALOG (colorsel_dialog)->
		       cancel_button), "clicked",
		      GTK_SIGNAL_FUNC (color_select_cancel_cb), NULL);
  gtk_signal_connect (GTK_OBJECT
		      (GTK_COLOR_SELECTION_DIALOG (colorsel_dialog)->
		       colorsel), "color_changed",
		      GTK_SIGNAL_FUNC (color_changed_cb), button);

  gtk_widget_show (colorsel_dialog);
  gtk_grab_add (colorsel_dialog);
  gtk_window_set_transient_for (GTK_WINDOW (colorsel_dialog),
				GTK_WINDOW (app.main_window));
}

/* 
 * FileType Page Callbacks
 */
static void
filetypes_select_row_cb (GtkWidget * clist, gint row, gint col,
			 GdkEvent * event)
{
  gchar label_text[NAME_MAX];
  gchar *program, *s;
  FileType *ft = gtk_clist_get_row_data (GTK_CLIST (clist), row);

  program = s = strdup (ft->programs);
  if ((s = strchr (program, ',')) != NULL)
    *s = '\0';

  g_snprintf (label_text, sizeof (label_text), "Default Action: %s", program);
  gtk_label_set_text (GTK_LABEL (filetypes_page.default_program_label),
		      label_text);
  free (program);
}

static void
add_filetype_cb (GtkWidget * button, GtkWidget * clist)
{
  create_init_filetype_dialog (NULL, clist);
}

static void
edit_filetype_cb (GtkWidget * button, GtkWidget * clist)
{
  gint selected_row;
  FileType *ft;

  if (GTK_CLIST (clist)->selection != NULL)
    selected_row = (gint) GTK_CLIST (clist)->selection->data;
  else
    return;

  ft = gtk_clist_get_row_data (GTK_CLIST (clist), selected_row);
  create_filetype_dialog (ft, FALSE);
}

static void
remove_filetype_cb (GtkWidget * button, GtkWidget * clist)
{
  gint selected_row;
  FileType *ft;

  if (GTK_CLIST (clist)->selection != NULL)
    selected_row = (gint) GTK_CLIST (clist)->selection->data;
  else
    return;

  ft = gtk_clist_get_row_data (GTK_CLIST (clist), selected_row);
  cfg.filetypes = g_list_remove (cfg.filetypes, ft);
  g_free (ft);

  gtk_clist_remove (GTK_CLIST (clist), selected_row);
}

/*
 * User Commands Page Callbacks
 */
static void
add_user_command_cb (GtkWidget * widget)
{
  UserCommand *command = NULL;
  create_user_command_dialog (&command);
  gtk_main ();
  if (command != NULL)
    {
      gchar *buf[1] = { command->name };
      gint row;
      row = gtk_clist_append (GTK_CLIST (user_commands_page.clist), buf);
      gtk_clist_set_row_data (GTK_CLIST (user_commands_page.clist), row,
			      command);
    }
}

static void
edit_user_command_cb (GtkWidget * widget)
{
  gint selected_row;
  UserCommand *command;

  if (GTK_CLIST (user_commands_page.clist)->selection != NULL)
    selected_row =
      (gint) GTK_CLIST (user_commands_page.clist)->selection->data;
  else
    return;

  command = gtk_clist_get_row_data (GTK_CLIST (user_commands_page.clist),
				    selected_row);
  create_user_command_dialog (&command);
  gtk_main ();
  gtk_clist_set_text (GTK_CLIST (user_commands_page.clist), selected_row, 0,
		      command->name);
}

static void
remove_user_command_cb (GtkWidget * widget)
{
  gint selected_row;
  UserCommand *command;

  if (GTK_CLIST (user_commands_page.clist)->selection != NULL)
    selected_row =
      (gint) GTK_CLIST (user_commands_page.clist)->selection->data;
  else
    return;

  command = gtk_clist_get_row_data (GTK_CLIST (user_commands_page.clist),
				    selected_row);
  cfg.user_commands = g_list_remove (cfg.user_commands, command);
  g_free (command);

  gtk_clist_remove (GTK_CLIST (user_commands_page.clist), selected_row);
}
/*
 * Programs Page Callbacks
 */
static void
add_program_cb (GtkWidget * widget)
{
  Programs *program = NULL;
  create_programs_dialog (&program);
  gtk_main ();
  if (program != NULL)
    {
      gchar *buf[1] = { program->name };
      gint row;
      row = gtk_clist_append (GTK_CLIST (programs_page.clist), buf);
      gtk_clist_set_row_data (GTK_CLIST (programs_page.clist), row,
			      program);
    }
}

static void
edit_programs_cb (GtkWidget * widget)
{
  gint selected_row;
  Programs *program;

  if (GTK_CLIST (programs_page.clist)->selection != NULL)
    selected_row =
      (gint) GTK_CLIST (programs_page.clist)->selection->data;
  else
    return;

  program = gtk_clist_get_row_data (GTK_CLIST (programs_page.clist),
				    selected_row);
  create_programs_dialog (&program);
  gtk_main ();
  gtk_clist_set_text (GTK_CLIST (programs_page.clist), selected_row, 0,
		      program->name);
}

static void
remove_program_cb (GtkWidget * widget)
{
  gint selected_row;
  Programs *program;

  if (GTK_CLIST (programs_page.clist)->selection != NULL)
    selected_row =
      (gint) GTK_CLIST (programs_page.clist)->selection->data;
  else
    return;

  program = gtk_clist_get_row_data (GTK_CLIST (programs_page.clist),
				    selected_row);
  cfg.programs = g_list_remove (cfg.programs, program);
  g_free (program);

  gtk_clist_remove (GTK_CLIST (programs_page.clist), selected_row);
}

static void
config_clist_select_row_cb (GtkWidget * clist)
{
  GtkWidget *notebook;
  gint page;

  if (GTK_CLIST (clist)->selection != NULL)
    page = (gint) GTK_CLIST (clist)->selection->data;
  else
    return;

  notebook = GTK_WIDGET (gtk_object_get_user_data (GTK_OBJECT (clist)));
  gtk_notebook_set_page (GTK_NOTEBOOK (notebook), page);

  if (page == 2)		/* bookmarks */
    {
      gtk_signal_connect (GTK_OBJECT (bookmarks_page.clist),
			  "key_press_event",
			  GTK_SIGNAL_FUNC (config_key_press_cb), NULL);
      gtk_clist_select_row (GTK_CLIST (bookmarks_page.clist), 0, 0);
      return;
    }
  else if (page == 3)		/* filetypes */
    {
      gtk_signal_connect (GTK_OBJECT (filetypes_page.clist),
			  "key_press_event",
			  GTK_SIGNAL_FUNC (config_key_press_cb), NULL);
      gtk_clist_select_row (GTK_CLIST (filetypes_page.clist), 0, 0);

      return;
    }
  else
    {
      return;
    }

}

/*
 * term options callbacks
 */

static void
save_font(GtkWidget *widget, GtkFontSelectionDialog *fontsel)
{
	gchar buf[512];
	
	if(g_snprintf(buf, sizeof(buf),
			gtk_font_selection_dialog_get_font_name(fontsel)))
	{
		g_snprintf(tmp_values.font, sizeof(tmp_values.font), buf);
		gtk_entry_set_text(GTK_ENTRY(zvt_general.term_font), tmp_values.font);
		gtk_editable_set_position(GTK_EDITABLE(zvt_general.term_font), 0);
	}
	tmp_values.font_change = 1;
	
	gtk_widget_destroy(GTK_WIDGET(fontsel));
	if( 1 < gtk_main_level())
		gtk_main_quit();

}

static void
font_dialog(GtkWidget *widget, gpointer data)
{
	GtkWidget *font_selector;
	gchar *spacings[] = {"c", "m", NULL};


	gtk_grab_remove(dialog);
	font_selector = gtk_font_selection_dialog_new("Choose Font");

	gtk_font_selection_dialog_set_filter(GTK_FONT_SELECTION_DIALOG(font_selector),
			GTK_FONT_FILTER_BASE, GTK_FONT_ALL,
			NULL, NULL, NULL, NULL, spacings, NULL);
	gtk_font_selection_dialog_set_font_name(
				GTK_FONT_SELECTION_DIALOG(font_selector), zvt.term_font);

	gtk_signal_connect(GTK_OBJECT
			(GTK_FONT_SELECTION_DIALOG(font_selector)->ok_button),
			"clicked", GTK_SIGNAL_FUNC(save_font), font_selector);
	gtk_signal_connect_object(GTK_OBJECT
			(GTK_FONT_SELECTION_DIALOG(font_selector)->cancel_button),
			"destroy", GTK_SIGNAL_FUNC(gtk_widget_destroy),
			 GTK_OBJECT(font_selector));

	gtk_signal_connect_object(GTK_OBJECT
			(GTK_FONT_SELECTION_DIALOG(font_selector)->cancel_button),
			"clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy),
			 GTK_OBJECT(font_selector));
	gtk_widget_show(GTK_WIDGET(font_selector));

}
/*
static void
file_select_ok_cb(GtkWidget *widget, GtkFileSelection *fileselect)
{
	tmp_values.pixmap_change = 1;
	g_snprintf(tmp_values.pixmap, sizeof(tmp_values.pixmap),
			gtk_file_selection_get_filename(GTK_FILE_SELECTION(fileselect)));
	gtk_entry_set_text(GTK_ENTRY(zvt_general.pixmap_entry), 
			tmp_values.pixmap);
	gtk_editable_set_position(GTK_EDITABLE(zvt_general.pixmap_entry), 0);

	gtk_widget_destroy(GTK_WIDGET(fileselect));
}

static  void
file_select_cancel(GtkWidget *widget, GtkFileSelection *fileselect)
{
	gtk_widget_destroy(GTK_WIDGET(fileselect));
}

static void
file_select_cb(GtkWidget *widget, gpointer data)
{
	GtkWidget *fileselect = gtk_file_selection_new("Choose a Background Image");


	gtk_grab_remove(dialog);
	gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(fileselect));

	gtk_signal_connect(GTK_OBJECT(fileselect), "destroy",
			GTK_SIGNAL_FUNC(file_select_cancel), fileselect);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fileselect)->ok_button),
			"clicked", GTK_SIGNAL_FUNC(file_select_ok_cb), fileselect);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fileselect)->cancel_button),
			"clicked", GTK_SIGNAL_FUNC(file_select_cancel), fileselect);
	gtk_widget_show(fileselect);
}
*/

static GtkWidget *
add_page (GtkWidget * notebook, gchar * label_text, GtkWidget * clist)
{
  GtkWidget *frame;
  GtkWidget *label;
  GtkWidget *vbox;
  GtkWidget *vvbox;
  gchar *buf[1];

  buf[0] = label_text;
  gtk_clist_append (GTK_CLIST (clist), buf);

  vvbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vvbox);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
  gtk_box_pack_start (GTK_BOX (vvbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);

  label = gtk_label_new (label_text);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label), 2, 1);
  gtk_container_add (GTK_CONTAINER (frame), label);
  gtk_widget_show (label);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_container_add (GTK_CONTAINER (vvbox), vbox);
  gtk_widget_show (vbox);

  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vvbox, NULL);
  return vbox;
}

void
option_menu_cb(GtkMenuItem *item, gpointer data)
{
	gint v;

	switch(GPOINTER_TO_INT(data))
	{
		case LINUX: /* Color Scheme */
			{
				for(v = 2; v < 18; v++)
				{
					gtk_widget_set_sensitive(GTK_WIDGET(zvt_general.color_button[v]),
							FALSE);
				}

				tmp_values.color_change = 1;
				g_snprintf(tmp_values.color_scheme, 
						sizeof(tmp_values.color_scheme), "Linux");
			}
		 break;
		case XTERM:
		 {
				for(v = 2; v < 18; v++)
				{
					gtk_widget_set_sensitive(GTK_WIDGET(zvt_general.color_button[v]),
							FALSE);
				}

			 tmp_values.color_change = 1;
		 	 g_snprintf(tmp_values.color_scheme, 
					 sizeof(tmp_values.color_scheme), "Xterm");
		 }
			break;
		case RXVT:
			{
				for(v = 2; v < 18; v++)
				{
					gtk_widget_set_sensitive(GTK_WIDGET(zvt_general.color_button[v]),
							FALSE);
				}

				tmp_values.color_change = 1;
				g_snprintf(tmp_values.color_scheme, 
						sizeof(tmp_values.color_scheme), "Rxvt");
			}
			break;
		case CUSTOM:
			{
				for(v = 2; v < 18; v++)
				{
					gtk_widget_set_sensitive(GTK_WIDGET(zvt_general.color_button[v]),
							TRUE);
				}

				tmp_values.color_change = 1;
				g_snprintf(tmp_values.color_scheme,
						sizeof(tmp_values.color_scheme), "Custom");
			}
			break;
		case WHITE_ON_BLACK: /* Background */
			{
				for(v = 0; v < 2; v++)
				{
					gtk_widget_set_sensitive(GTK_WIDGET(zvt_general.color_button[v]),
							FALSE);
				}

				tmp_values.background_change = 1;
				g_snprintf(tmp_values.background, sizeof(zvt.background), "WhiteOnBlack");
			}
			break;
		case BLACK_ON_WHITE:
			{
				for(v = 0; v < 2; v++)
				{
					gtk_widget_set_sensitive(GTK_WIDGET(zvt_general.color_button[v]),
							FALSE);
				}

				tmp_values.background_change = 1;
				g_snprintf(tmp_values.background, sizeof(zvt.background), "BlackOnWhite");
			}
			break;
		case GREEN_ON_BLACK:
			{
				for(v = 0; v < 2; v++)
				{
					gtk_widget_set_sensitive(GTK_WIDGET(zvt_general.color_button[v]),
							FALSE);
				}

				tmp_values.background_change = 1;
				g_snprintf(tmp_values.background, sizeof(zvt.background), "GreenOnBlack");
			}
			break;
		case CUSTOM_BACK:
			{
				for(v = 0; v < 2; v++)
				{
					gtk_widget_set_sensitive(GTK_WIDGET(zvt_general.color_button[v]),
							TRUE);
				}

				tmp_values.background_change = 1;
				g_snprintf(tmp_values.background, sizeof(zvt.background), "CustomBack");
	
			}
			break;
		case LEFT: /* notebook tab */
			{
				tmp_values.tab_change = 1;
				tmp_values.tab_position = GTK_POS_LEFT;
				tmp_values.show_tabs = TRUE;
			}
				break;
		case RIGHT:
				{
					tmp_values.tab_change = 1;
					tmp_values.tab_position = GTK_POS_RIGHT;
					tmp_values.show_tabs = TRUE;
				}
				break;
		case TOP:
				{
					tmp_values.tab_change = 1;
					tmp_values.tab_position = GTK_POS_TOP;
					tmp_values.show_tabs = TRUE;
				}
				break;
		case BOTTOM:
				{
					tmp_values.tab_change = 1;
					tmp_values.tab_position = GTK_POS_BOTTOM;
					tmp_values.show_tabs = TRUE;
				}
				break;
		case HIDDEN:
				{
					tmp_values.hide_change = 1;
					tmp_values.show_tabs = FALSE;
				}
				break;
		default:
			break;
	}
}

void
create_config_dialog (gint page)
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *notebook;
  GtkWidget *vbox;
  GtkWidget *sub_vbox;
  GtkWidget *hbox;
  GtkWidget *frame;
  GtkWidget *table;
  GtkWidget *sw;
  GtkWidget *config_clist;
  GtkWidget *wid;
	GtkWidget *menu;
	GtkWidget *menuitem;
  GtkStyle *style;
  gchar *titles[2];
  gchar *buf[2];
  gchar label_text[MAX_LEN];
  GList *tmp;
  int i, z;
	gint x;
	gint left = 0;
	gint right = 1;
	gint top = 1;
	gint bottom = 2;
	gint y = 0;

	GtkWidget *font_button;
	tmp_values.color_change = 0;
	tmp_values.background_change = 0;
	tmp_values.tab_change = 0;
	tmp_values.pixmap_change = 0;
	tmp_values.font_change = 0;

  dialog = gtk_dialog_new ();

  dialog_vbox = GTK_DIALOG (dialog)->vbox;
  action_area = GTK_DIALOG (dialog)->action_area;
  gtk_widget_set_usize (dialog, 450, 350);
  gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox), 5);
  gtk_box_set_spacing (GTK_BOX (dialog_vbox), 5);
  gtk_container_set_border_width (GTK_CONTAINER (action_area), 5);
  add_button (action_area, "OK", TRUE, 0, ok_cb, NULL);
  add_button (action_area, "Cancel", TRUE, 0, cancel_cb, NULL);
  gtk_signal_connect (GTK_OBJECT (dialog), "key_press_event",
		      GTK_SIGNAL_FUNC (key_press_cb), NULL);

  /* the main hbox - contains the clist and the frame */
  hbox = add_hbox (dialog_vbox, FALSE, 5, TRUE, 0);

  /* the config clist */
  titles[0] = "Configuration";
  config_clist = gtk_clist_new_with_titles (1, titles);
  gtk_signal_connect (GTK_OBJECT (config_clist), "key_press_event",
		      GTK_SIGNAL_FUNC (config_key_press_cb), NULL);
  gtk_clist_set_selection_mode (GTK_CLIST (config_clist),
				GTK_SELECTION_BROWSE);
  gtk_clist_column_titles_passive (GTK_CLIST (config_clist));
  gtk_signal_connect (GTK_OBJECT (config_clist), "select_row",
		      GTK_SIGNAL_FUNC (config_clist_select_row_cb), NULL);
  gtk_widget_set_usize (config_clist, 120, 0);
  gtk_box_pack_start (GTK_BOX (hbox), config_clist, FALSE, FALSE, 0);
  gtk_widget_show (config_clist);

  /* the main frame - holds the notebook */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  /* the notebook */
  notebook = gtk_notebook_new ();
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), FALSE);
  gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook), FALSE);
  gtk_container_add (GTK_CONTAINER (frame), notebook);
  gtk_object_set_user_data (GTK_OBJECT (config_clist), notebook);
  gtk_widget_show (notebook);

  /* General Tab */
  vbox = add_page (notebook, "General - Page 1", config_clist);

  frame = gtk_frame_new ("Directories");
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 5);
  gtk_widget_show (frame);

  table = gtk_table_new (3, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  add_label_to_table (table, "Left Startup Dir: ", 0, 0, 1, 0, 1);
  general_page_1.left_startup_dir_entry = add_entry_to_table (table,
							      cfg.
							      left_startup_dir,
							      1, 2, 0, 1);
  add_label_to_table (table, "Right Startup Dir: ", 0, 0, 1, 1, 2);
  general_page_1.right_startup_dir_entry = add_entry_to_table (table,
							       cfg.
							       right_startup_dir,
							       1, 2, 1, 2);
	general_page_1.start_with_cwd_checkbutton = add_check_button_to_table(
			table, "Startup in Current Working Directory",
			cfg.start_with_cwd, start_with_cwd_cb,
			NULL, 0, 2, 2, 3);
	if(GTK_TOGGLE_BUTTON(general_page_1.start_with_cwd_checkbutton)->active)
	{
		gtk_widget_set_sensitive(general_page_1.left_startup_dir_entry, FALSE);
		gtk_widget_set_sensitive(general_page_1.right_startup_dir_entry, FALSE);
	}

	if(GTK_TOGGLE_BUTTON(general_page_1.start_with_cwd_checkbutton)->active)
	{
		gtk_widget_set_sensitive(general_page_1.left_startup_dir_entry, FALSE);
		gtk_widget_set_sensitive(general_page_1.right_startup_dir_entry, FALSE);
	}

	frame = gtk_frame_new ("Choose your favorite vi clone");
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 5);
  gtk_widget_show (frame);

	sub_vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), sub_vbox);
  gtk_widget_show (sub_vbox);

	general_page_1.vim = gtk_radio_button_new_with_label(NULL, "Vim");
	gtk_widget_show(general_page_1.vim);
	gtk_box_pack_start(GTK_BOX(sub_vbox), general_page_1.vim, TRUE, TRUE, 0);
	if(!strcmp("vim", cfg.vi_clone))
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(general_page_1.vim), TRUE);
	general_page_1.vile = 
		gtk_radio_button_new_with_label_from_widget(
				GTK_RADIO_BUTTON(general_page_1.vim),
				"Vile");
	gtk_widget_show(general_page_1.vile);
	gtk_box_pack_start(GTK_BOX(sub_vbox), general_page_1.vile,
				TRUE, TRUE, 0);
	if(!strcmp("vile", cfg.vi_clone))
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(general_page_1.vile), TRUE);
	general_page_1.elvis = 
		gtk_radio_button_new_with_label_from_widget(
				GTK_RADIO_BUTTON(general_page_1.vim),
				"Elvis");
	gtk_widget_show(general_page_1.elvis);
	gtk_box_pack_start(GTK_BOX(sub_vbox), general_page_1.elvis,
				TRUE, TRUE, 0);
	if(!strcmp("elvis", cfg.vi_clone))
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(general_page_1.elvis), TRUE);
	general_page_1.nvi = 
		gtk_radio_button_new_with_label_from_widget(
				GTK_RADIO_BUTTON(general_page_1.vim),
				"Nvi");
	gtk_widget_show(general_page_1.nvi);
	gtk_box_pack_start(GTK_BOX(sub_vbox), general_page_1.nvi,
				TRUE, TRUE, 0);
	if(!strcmp("vi", cfg.vi_clone))
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(general_page_1.nvi), TRUE);
	general_page_1.vi_entry = gtk_entry_new();
	gtk_widget_show(general_page_1.vi_entry);
	wid = gtk_label_new("Actual vi command used");
	gtk_widget_show(wid);
	gtk_box_pack_start(GTK_BOX(sub_vbox), wid, TRUE, TRUE, 0);
	gtk_entry_set_text(GTK_ENTRY(general_page_1.vi_entry), cfg.vi_command);
	gtk_box_pack_start(GTK_BOX(sub_vbox), general_page_1.vi_entry,
			TRUE, TRUE, 0);

  /* General - Page 2 */
  vbox = add_page (notebook, "General - Page 2", config_clist);

  frame = gtk_frame_new ("Histories");
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
  gtk_widget_show (frame);

  table = gtk_table_new (2, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  add_label_to_table (table, "Directory History Size: ", 0, 0, 1, 0, 1);
  g_snprintf (label_text, sizeof (label_text), "%d",
	      cfg.dir_history_max_length);
  general_page_2.dir_history_entry =
    add_entry_to_table (table, label_text, 1, 2, 0, 1);
  add_label_to_table (table, "Command History Size: ", 0, 0, 1, 1, 2);
  g_snprintf (label_text, sizeof (label_text), "%d",
	      cfg.command_history_max_length);
  general_page_2.command_history_entry =
    add_entry_to_table (table, label_text, 1, 2, 1, 2);

  frame = gtk_frame_new ("Misc");
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 5);
  gtk_widget_show (frame);

  table = gtk_table_new (3, 3, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

	general_page_2.use_scrollbars = add_check_button_to_table(table, 
			"Show Scrollbars", cfg.use_scrollbars, NULL, NULL,  0, 1, 0, 1);
	general_page_2.save_position = add_check_button_to_table(table,
			"Save Position", cfg.save_position, NULL, NULL, 0, 1, 1, 2);

  add_label_to_table (table, "Xterm Command: ", 0, 0, 1, 2, 3);
  general_page_2.xterm_entry = add_entry_to_table (table, cfg.xterm_command,
						   2, 3, 2, 3);

	frame = gtk_frame_new("Name Filter");
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, TRUE, 5);
	gtk_widget_show(frame);

	table = gtk_table_new(3, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  add_label_to_table (table, "File Name Pattern: ", 0, 0, 1, 0, 1);
  general_page_2.filter_entry = add_entry_to_table (table, cfg.start_filter,
						     1, 2, 0, 1);

	general_page_2.invert_filter_check_button = add_check_button_to_table(table,
			"Invert Pattern", cfg.invert_filter, NULL,
			NULL, 0, 1, 1, 2);



  /* Bookmarks Tab */
  vbox = add_page (notebook, "Bookmarks", config_clist);

  sw = add_sw (vbox, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC, TRUE, 0);

  titles[0] =  "Mark        ";
	titles[1] = "Directory             ";
	titles[2] = "File           ";

  bookmarks_page.clist = gtk_clist_new_with_titles (3, titles);

  gtk_clist_column_titles_passive (GTK_CLIST (bookmarks_page.clist));
  gtk_clist_set_reorderable (GTK_CLIST (bookmarks_page.clist), TRUE);
  gtk_clist_set_selection_mode (GTK_CLIST (bookmarks_page.clist),
				GTK_SELECTION_EXTENDED);
  gtk_signal_connect (GTK_OBJECT (bookmarks_page.clist), "select_row",
		      GTK_SIGNAL_FUNC (bookmarks_select_row_cb), NULL);
  gtk_container_add (GTK_CONTAINER (sw), bookmarks_page.clist);
	for(tmp = cfg.bookmarks; tmp != NULL; tmp = tmp->next)
	{
		Bookmarks *bk = tmp->data;

		buf[0] = bk->mark;
		buf[1] = bk->dir;
		buf[2] = bk->file;

		i = gtk_clist_append(GTK_CLIST(bookmarks_page.clist), buf);
		gtk_clist_set_row_data(GTK_CLIST(bookmarks_page.clist), i, bk);
	}

  gtk_widget_show (bookmarks_page.clist);

  hbox = add_hbox (vbox, TRUE, 0, FALSE, 5);
  add_button (hbox, "Up", TRUE, 5, bookmark_up_cb, bookmarks_page.clist);
  add_button (hbox, "Down", TRUE, 5, bookmark_down_cb, bookmarks_page.clist);
  add_button (hbox, "Remove", TRUE, 5, bookmark_remove_cb,
	      bookmarks_page.clist);

  /* Filetypes Tab */
  vbox = add_page (notebook, "Filetypes", config_clist);

  sw = add_sw (vbox, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC, TRUE, 0);

  titles[0] = "Extensions";
  titles[1] = "Description";
  filetypes_page.clist = gtk_clist_new_with_titles (2, titles);
  gtk_clist_column_titles_passive (GTK_CLIST (filetypes_page.clist));
  gtk_clist_set_reorderable (GTK_CLIST (filetypes_page.clist), TRUE);
  gtk_clist_set_selection_mode (GTK_CLIST (filetypes_page.clist),
				GTK_SELECTION_EXTENDED);
  gtk_signal_connect (GTK_OBJECT (filetypes_page.clist), "select_row",
		      GTK_SIGNAL_FUNC (filetypes_select_row_cb), NULL);
	/**** change selection mode here *****************************/
  gtk_container_add (GTK_CONTAINER (sw), filetypes_page.clist);
  gtk_widget_show (filetypes_page.clist);

  for (tmp = cfg.filetypes; tmp != NULL; tmp = tmp->next)
    {
      FileType *ft = tmp->data;

      buf[0] = ft->extensions;
      buf[1] = ft->description;

      i = gtk_clist_append (GTK_CLIST (filetypes_page.clist), buf);
      gtk_clist_set_row_data (GTK_CLIST (filetypes_page.clist), i, ft);
    }

  hbox = add_hbox (vbox, FALSE, 0, FALSE, 2);
  filetypes_page.default_program_label = add_label (hbox, "Default Action: ",
						    0.01, TRUE, 0);

  hbox = add_hbox (vbox, TRUE, 0, FALSE, 5);
  add_button (hbox, "Add", TRUE, 5, add_filetype_cb, filetypes_page.clist);
  add_button (hbox, "Edit", TRUE, 5, edit_filetype_cb, filetypes_page.clist);
  add_button (hbox, "Remove", TRUE, 5, remove_filetype_cb,
	      filetypes_page.clist);

  /* User Commands Tab */
  vbox = add_page (notebook, "User Commands", config_clist);

  wid = add_label (vbox, ""
		   , 0.0, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (wid), GTK_JUSTIFY_LEFT);

  sw = add_sw (vbox, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC, TRUE, 0);

  titles[0] = "Name";
  user_commands_page.clist = gtk_clist_new_with_titles (1, titles);
  gtk_clist_column_titles_passive (GTK_CLIST (user_commands_page.clist));
  gtk_clist_set_reorderable (GTK_CLIST (user_commands_page.clist), TRUE);
  gtk_container_add (GTK_CONTAINER (sw), user_commands_page.clist);
  gtk_widget_show (user_commands_page.clist);
  gtk_signal_connect (GTK_OBJECT (user_commands_page.clist),
		      "key_press_event",
		      GTK_SIGNAL_FUNC (config_key_press_cb), NULL);
  gtk_clist_set_selection_mode (GTK_CLIST (user_commands_page.clist),
				GTK_SELECTION_BROWSE);


  for (tmp = cfg.user_commands; tmp != NULL; tmp = tmp->next)
    {
      UserCommand *command = tmp->data;

      buf[0] = command->name;

      i = gtk_clist_append (GTK_CLIST (user_commands_page.clist), buf);
      gtk_clist_set_row_data (GTK_CLIST (user_commands_page.clist), i,
			      command);
    }

  hbox = add_hbox (vbox, TRUE, 0, FALSE, 5);
  add_button (hbox, "Add", TRUE, 5, add_user_command_cb,
	      user_commands_page.clist);
  add_button (hbox, "Edit", TRUE, 5, edit_user_command_cb,
	      user_commands_page.clist);
  add_button (hbox, "Remove", TRUE, 5, remove_user_command_cb,
	      user_commands_page.clist);

  /* Columns Tab */
  vbox = add_page (notebook, "Columns", config_clist);

  sw = add_sw (vbox, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC, TRUE, 0);

  sub_vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (sub_vbox);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sw), sub_vbox);

  for (i = 0; i < MAX_COLUMNS; i++)
    {
      columns_page.checks[i] =
	add_check_button (sub_vbox, all_columns[i].title, 0, FALSE, 0, NULL,
			  NULL);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				    (columns_page.checks[i]),
				    all_columns[i].is_visible);
    }
  gtk_widget_set_sensitive (columns_page.checks[FILENAME], FALSE);

  /* Colors Tab */
  vbox = add_page (notebook, "File Colors", config_clist);

  table = add_table (vbox, 7, 2, FALSE, FALSE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);

  add_label_to_table (table, "Active List Column Button", 0, 0, 1, 0, 1);
  wid =
    add_button_to_table (table, "", color_select_cb, &COL_COLOR, 1, 2, 0, 1);
  style = gtk_style_new ();
  style->bg[GTK_STATE_NORMAL] = COL_COLOR;
  gtk_widget_set_style (wid, style);

  add_label_to_table (table, "Executable", 0, 0, 1, 1, 2);
  wid =
    add_button_to_table (table, "", color_select_cb, &EXE_COLOR, 1, 2, 1, 2);
  style = gtk_style_new ();
  style->bg[GTK_STATE_NORMAL] = EXE_COLOR;
  gtk_widget_set_style (wid, style);

  add_label_to_table (table, "Symbolic Link", 0, 0, 1, 2, 3);
  wid =
    add_button_to_table (table, "", color_select_cb, &LNK_COLOR, 1, 2, 2, 3);
  style = gtk_style_new ();
  style->bg[GTK_STATE_NORMAL] = LNK_COLOR;
  gtk_widget_set_style (wid, style);

  add_label_to_table (table, "Special Device", 0, 0, 1, 3, 4);
  wid =
    add_button_to_table (table, "", color_select_cb, &DEV_COLOR, 1, 2, 3, 4);
  style = gtk_style_new ();
  style->bg[GTK_STATE_NORMAL] = DEV_COLOR;
  gtk_widget_set_style (wid, style);

  add_label_to_table (table, "Socket", 0, 0, 1, 4, 5);
  wid =
    add_button_to_table (table, "", color_select_cb, &SOCK_COLOR, 1, 2, 4, 5);
  style = gtk_style_new ();
  style->bg[GTK_STATE_NORMAL] = SOCK_COLOR;
  gtk_widget_set_style (wid, style);

  add_label_to_table (table, "Directory", 0, 0, 1, 5, 6);
  wid =
    add_button_to_table (table, "", color_select_cb, &DIR_COLOR, 1, 2, 5, 6);
  style = gtk_style_new ();
  style->bg[GTK_STATE_NORMAL] = DIR_COLOR;
  gtk_widget_set_style (wid, style);

  add_label_to_table (table, "Tagged", 0, 0, 1, 6, 7);
  wid =
    add_button_to_table (table, "", color_select_cb, &TAG_COLOR, 1, 2, 6, 7);
  style = gtk_style_new ();
  style->bg[GTK_STATE_NORMAL] = TAG_COLOR;
  gtk_widget_set_style (wid, style);

  gtk_widget_show (table);


  gtk_clist_select_row (GTK_CLIST (config_clist), page, 0);

  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_widget_show (dialog);
  gtk_grab_add (dialog);
  gtk_window_set_transient_for (GTK_WINDOW (dialog),
				GTK_WINDOW (app.main_window));
  gtk_widget_grab_focus (GTK_WIDGET (config_clist));

  /* Programs Tab */
  vbox = add_page (notebook, "Programs", config_clist);

  wid = add_label (vbox, "    Popup menu activated by pressing P \n "
		   , 0.0, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (wid), GTK_JUSTIFY_LEFT);

  sw = add_sw (vbox, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC, TRUE, 0);

  titles[0] = "Name";
  programs_page.clist = gtk_clist_new_with_titles (1, titles);
  gtk_clist_column_titles_passive (GTK_CLIST (programs_page.clist));
  gtk_clist_set_reorderable (GTK_CLIST (programs_page.clist), TRUE);
  gtk_container_add (GTK_CONTAINER (sw), programs_page.clist);
  gtk_widget_show (programs_page.clist);
  gtk_signal_connect (GTK_OBJECT (programs_page.clist),
		      "key_press_event",
		      GTK_SIGNAL_FUNC (config_key_press_cb), NULL);
  gtk_clist_set_selection_mode (GTK_CLIST (programs_page.clist),
				GTK_SELECTION_BROWSE);


  for (tmp = cfg.programs; tmp != NULL; tmp = tmp->next)
    {
      Programs *program = tmp->data;

      buf[0] = program->name;

      i = gtk_clist_append (GTK_CLIST (programs_page.clist), buf);
      gtk_clist_set_row_data (GTK_CLIST (programs_page.clist), i,
			      program);
    }

  hbox = add_hbox (vbox, TRUE, 0, FALSE, 5);
  add_button (hbox, "Add", TRUE, 5, add_program_cb,
	      programs_page.clist);
  add_button (hbox, "Edit", TRUE, 5, edit_programs_cb,
	      programs_page.clist);
  add_button (hbox, "Remove", TRUE, 5, remove_program_cb,
	      programs_page.clist);
	g_list_free(tmp);

	/* Zvt General */
  vbox = add_page (notebook, "Term Options 1", config_clist);
  sw = add_sw (vbox, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC, TRUE, 0);

  sub_vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (sub_vbox);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sw), sub_vbox);

  frame = gtk_frame_new ("Terminal Font");
	gtk_widget_show(frame);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 2);
	gtk_box_pack_start(GTK_BOX(sub_vbox), frame, TRUE, TRUE, 0);
	hbox = gtk_hbox_new(FALSE, TRUE);
	gtk_widget_show(hbox);
	gtk_container_add(GTK_CONTAINER(frame), hbox);

	zvt_general.term_font = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(zvt_general.term_font), zvt.term_font);
	gtk_editable_set_position(GTK_EDITABLE(zvt_general.term_font), 0);
	gtk_widget_show(zvt_general.term_font);
	gtk_box_pack_start(GTK_BOX(hbox), zvt_general.term_font, FALSE, TRUE, 0);
	
	font_button = gtk_button_new_with_label("Browse");
	gtk_widget_show(font_button);
	gtk_box_pack_start(GTK_BOX(hbox), font_button, TRUE, TRUE, 0);
	gtk_signal_connect_object(GTK_OBJECT(font_button), "clicked",
			GTK_SIGNAL_FUNC(font_dialog), NULL);

	frame = gtk_frame_new ("Misc");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 2);
  gtk_box_pack_start (GTK_BOX (sub_vbox), frame, FALSE, TRUE, 5);
  gtk_widget_show (frame);
  table = gtk_table_new (2, 7, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (table), 1);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  add_label_to_table (table, "  Notebook Tab Position ", 0, 0, 1, 0, 1);
	zvt_general.tab_menu = gtk_option_menu_new();
	menu = gtk_menu_new();
		menuitem = gtk_menu_item_new_with_label("Left");
	gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
			GTK_SIGNAL_FUNC(option_menu_cb), GUINT_TO_POINTER(LEFT));
	gtk_widget_show(menuitem);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	menuitem = gtk_menu_item_new_with_label("Right");
	gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
			GTK_SIGNAL_FUNC(option_menu_cb), GUINT_TO_POINTER(RIGHT));
	gtk_widget_show(menuitem);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	menuitem = gtk_menu_item_new_with_label("Top");
	gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
			GTK_SIGNAL_FUNC(option_menu_cb), GUINT_TO_POINTER(TOP));
	gtk_widget_show(menuitem);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	menuitem = gtk_menu_item_new_with_label("Bottom");
	gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
			GTK_SIGNAL_FUNC(option_menu_cb), GUINT_TO_POINTER(BOTTOM));
	gtk_widget_show(menuitem);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	menuitem = gtk_menu_item_new_with_label("Hidden");
	gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
			GTK_SIGNAL_FUNC(option_menu_cb), GUINT_TO_POINTER(HIDDEN));
	gtk_widget_show(menuitem);
	gtk_menu_append(GTK_MENU(menu), menuitem);

	gtk_option_menu_set_menu(GTK_OPTION_MENU(zvt_general.tab_menu), menu);
	if(!zvt.show_tabs)
		gtk_option_menu_set_history(GTK_OPTION_MENU(zvt_general.tab_menu), 4);
	else
		gtk_option_menu_set_history(GTK_OPTION_MENU(zvt_general.tab_menu), 
				zvt.tab_position);

	gtk_widget_show(zvt_general.tab_menu);
	gtk_table_attach_defaults(GTK_TABLE(table), zvt_general.tab_menu, 1, 2, 0, 1);

  add_label_to_table (table, "  Scrollback Lines ", 0, 0, 1, 1, 2);

	zvt_general.adj = gtk_adjustment_new(100, 0, 1500, 1, 10, 10);
	zvt_general.spin = gtk_spin_button_new(GTK_ADJUSTMENT(zvt_general.adj), 10, 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(zvt_general.spin),
			zvt.scroll_lines);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(zvt_general.spin), TRUE);
	gtk_widget_show(zvt_general.spin);
	gtk_table_attach_defaults(GTK_TABLE(table), zvt_general.spin, 1, 2, 1, 2);
	
	zvt_general.blinking_cursor = add_check_button_to_table(
			table, "Use Blinking Cursor",
			zvt.use_blinking_cursor, NULL,
			NULL, 0, 1, 2, 3);
	zvt_general.term_bell = add_check_button_to_table(
			table, "Enable Terminal Bell",
			zvt.terminal_bell, NULL,
			NULL, 0, 1, 3, 4);
	zvt_general.scroll_key = add_check_button_to_table(
			table, "Scroll on Keystroke",
			zvt.scroll_on_keystroke, NULL,
			NULL, 0, 1, 4, 5);
	zvt_general.scroll_output = add_check_button_to_table(
			table, "Scroll on Output",
			zvt.scroll_on_output, NULL,
			NULL, 0, 1, 5, 6);
	zvt_general.swap = add_check_button_to_table(
			table, "Swap Backspace/Delete",
			zvt.swap_backspace, NULL,
			NULL, 0, 1, 6, 7);

	/* Term Background */
  vbox = add_page (notebook, "Term Options 2", config_clist);

	/*
  frame = gtk_frame_new ("Background Pixmap");
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 5);
  gtk_widget_show (frame);
  table = gtk_table_new (2, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);
  zvt_general.pixmap_entry = add_entry_to_table (table,
							      zvt.pixmap,
							      0, 1, 0, 1);
 add_button_to_table (table, "Browse", file_select_cb, NULL, 1, 2, 0, 1);
	zvt_general.pixmap = add_check_button_to_table(
			table, "Use Background Pixmap",
			zvt.use_pixmap, NULL,
			NULL, 0, 1, 1, 2);
			*/
  frame = gtk_frame_new ("Transparent Background");
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 5);
  gtk_widget_show (frame);
  table = gtk_table_new (2, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);
 
	zvt_general.transparent = add_check_button_to_table(
			table, "Transparent",
			zvt.transparent, NULL,
			NULL, 0, 1, 0, 1);
	/*
	zvt_general.shaded = add_check_button_to_table(
			table, "Shaded",
			zvt.shaded, NULL,
			NULL, 0, 1, 1, 2);
			*/


  frame = gtk_frame_new ("Notebook Tab Color");
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 5);
  gtk_widget_show (frame);
  table = gtk_table_new (2, 3, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

  add_label_to_table (table, " Highlighted Tab ", 0, 0, 1, 0, 1);
	zvt_general.tab_button = add_button_to_table(table, " ",
			color_select_cb, &TAB_COLOR, 1, 2, 0, 1);
	style = gtk_style_new();
	style->bg[GTK_STATE_NORMAL] = TAB_COLOR;
	gtk_widget_set_style(zvt_general.tab_button, style);

  frame = gtk_frame_new ("Program to lookup word under term cursor");
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 5);
  gtk_widget_show (frame);
  table = gtk_table_new (2, 3, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);
	zvt_general.program_entry = add_entry_to_table(table, cfg.man_program,
			1, 2, 0, 1);
	gtk_widget_show(zvt_general.program_entry);


	/* Term custom colors */
  vbox = add_page (notebook, "Term Options 3", config_clist);
  frame = gtk_frame_new ("Foreground / Background Colors");
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 5);
  gtk_widget_show (frame);
  table = gtk_table_new (2, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

	zvt_general.background_menu = gtk_option_menu_new();
	menu = gtk_menu_new();
	menuitem = gtk_menu_item_new_with_label("White on Black");
	gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
			GTK_SIGNAL_FUNC(option_menu_cb), GUINT_TO_POINTER(WHITE_ON_BLACK));
	gtk_widget_show(menuitem);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	menuitem = gtk_menu_item_new_with_label("Black on White");
	gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
			GTK_SIGNAL_FUNC(option_menu_cb), GUINT_TO_POINTER(BLACK_ON_WHITE));
	gtk_widget_show(menuitem);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	menuitem = gtk_menu_item_new_with_label("Green on Black");
	gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
			GTK_SIGNAL_FUNC(option_menu_cb),  GUINT_TO_POINTER(GREEN_ON_BLACK));
	gtk_widget_show(menuitem);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	menuitem = gtk_menu_item_new_with_label("Custom");
	gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
			GTK_SIGNAL_FUNC(option_menu_cb),  GUINT_TO_POINTER(CUSTOM_BACK));
	gtk_widget_show(menuitem);
	gtk_menu_append(GTK_MENU(menu), menuitem);

	gtk_option_menu_set_menu(GTK_OPTION_MENU(zvt_general.background_menu), menu);
	gtk_widget_show(zvt_general.background_menu);
	gtk_table_attach_defaults(GTK_TABLE(table),
			zvt_general.background_menu, 0, 2, 0, 1);
	if(!strcmp(zvt.background, "GreenOnBlack"))
			i = 2;
	if(!strcmp(zvt.background, "BlackOnWhite"))
		i = 1;
	if(!strcmp(zvt.background, "WhiteOnBlack"))
		i = 0;
	else
		i = 3;

	gtk_option_menu_set_history(GTK_OPTION_MENU(zvt_general.background_menu), i);

  zvt_general.color_button[0] =
    add_button_to_table (table, "ForeGround", color_select_cb,
				&zvt.palette[16], 0, 1, 1, 2);
	if(i != 3)
		gtk_widget_set_sensitive(GTK_WIDGET(zvt_general.color_button[0]), FALSE);

  style = gtk_style_new ();
  style->bg[GTK_STATE_NORMAL] = zvt.palette[16];
  gtk_widget_set_style (zvt_general.color_button[0], style);


  zvt_general.color_button[1] =
    add_button_to_table (table, "Background", color_select_cb, 
				&zvt.palette[17], 1, 2, 1, 2);
	if(i != 3)
		gtk_widget_set_sensitive(GTK_WIDGET(zvt_general.color_button[1]), FALSE);

  style = gtk_style_new ();
  style->bg[GTK_STATE_NORMAL] = zvt.palette[17];
  gtk_widget_set_style (zvt_general.color_button[1], style);

  frame = gtk_frame_new ("Terminal Colors");
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 5);
  gtk_widget_show (frame);
  table = gtk_table_new (4, 5, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 1);
  gtk_container_add (GTK_CONTAINER (frame), table);
  gtk_widget_show (table);

	zvt_general.scheme_menu = gtk_option_menu_new();
	menu = gtk_menu_new();
	menuitem = gtk_menu_item_new_with_label("Linux");
	gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
			GTK_SIGNAL_FUNC(option_menu_cb), GUINT_TO_POINTER(LINUX));
	gtk_widget_show(menuitem);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	menuitem = gtk_menu_item_new_with_label("Xterm");
	gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
			GTK_SIGNAL_FUNC(option_menu_cb), GUINT_TO_POINTER(XTERM));
	gtk_widget_show(menuitem);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	menuitem = gtk_menu_item_new_with_label("Rxvt");
	gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
			GTK_SIGNAL_FUNC(option_menu_cb),  GUINT_TO_POINTER(RXVT));
	gtk_widget_show(menuitem);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	menuitem = gtk_menu_item_new_with_label("Custom");
	gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
			GTK_SIGNAL_FUNC(option_menu_cb),  GUINT_TO_POINTER(CUSTOM));
	gtk_widget_show(menuitem);
	gtk_menu_append(GTK_MENU(menu), menuitem);
	gtk_option_menu_set_menu(GTK_OPTION_MENU(zvt_general.scheme_menu), menu);
	gtk_widget_show(zvt_general.scheme_menu);
	gtk_table_attach_defaults(GTK_TABLE(table),
			zvt_general.scheme_menu, 0, 4, 0, 1);
	if(!strcmp(zvt.term_color, "Linux"))
		i = 0;
	if(!strcmp(zvt.term_color, "Xterm"))
		i = 1;
	if(!strcmp(zvt.term_color, "Rxvt"))
		i = 2;
	else
		i = 3;
	gtk_option_menu_set_history(GTK_OPTION_MENU(zvt_general.scheme_menu), i);


	for(z = 0; z < 4; z++)
	{
		for(x = 0; x < 4; x++)
		{
			gchar label[4];
			g_snprintf(label, sizeof(label), "%d", y +1);

	  	zvt_general.color_button[y +2] =
  	  	add_button_to_table (table, label, color_select_cb, &zvt.palette[y], 
						left, right, top, bottom);
			if(i != 3)
				gtk_widget_set_sensitive(GTK_WIDGET(zvt_general.color_button[y +2]),
						FALSE);

  		style = gtk_style_new ();
  		style->bg[GTK_STATE_NORMAL] = zvt.palette[y];
  		gtk_widget_set_style (zvt_general.color_button[y +2], style);
			left++;
			right++;
			y++;
		}
		left = 0;
		right = 1;
		top++;
		bottom++;
	}
}
