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
#include "vide.h"

static GtkWidget *init_filetype_dialog;
static GtkWidget *extensions_entry;
static GtkWidget *program_entry;
static GtkWidget *description_entry;

static void
init_filetype_ok_cb (GtkWidget * widget, GtkWidget * clist)
{
  gchar *program = gtk_entry_get_text (GTK_ENTRY (program_entry));
  gchar *description = gtk_entry_get_text (GTK_ENTRY (description_entry));
  gchar *extensions = gtk_entry_get_text (GTK_ENTRY (extensions_entry));

  if (STREQ (extensions, ""))
    {
      return;
    }
  if (STREQ (program, ""))
    {
      return;
    }
  if (STREQ (description, ""))	/* if no desc is given use the extensions */
    description = extensions;

  add_filetype (extensions, program, description);
  write_filetypes_file ();

  if (clist != NULL)
    {
      gchar *buf[2];
      GList *tmp;
      gtk_clist_clear (GTK_CLIST (clist));
      for (tmp = cfg.filetypes; tmp != NULL; tmp = tmp->next)
	{
	  FileType *ft = tmp->data;
	  int i;

	  buf[0] = ft->extensions;
	  buf[1] = ft->description;
	  i = gtk_clist_append (GTK_CLIST (clist), buf);
	  gtk_clist_set_row_data (GTK_CLIST (clist), i, ft);
	}
      gtk_grab_remove (init_filetype_dialog);
      gtk_widget_destroy (init_filetype_dialog);
      return;
    }

  exec_action (program);
  gtk_grab_remove (init_filetype_dialog);
  gtk_widget_destroy (init_filetype_dialog);
}

static void
add_ext_to_filetype_cb (GtkWidget * widget)
{
  gchar *ext = gtk_entry_get_text (GTK_ENTRY (extensions_entry));

  if (STREQ (ext, ""))
    {
      return;
    }

  create_add_ext_dialog (ext);
  gtk_grab_remove (init_filetype_dialog);
  gtk_widget_destroy (init_filetype_dialog);
}

static void
cancel_cb (GtkWidget * widget, GtkWidget * dialog)
{
  gtk_grab_remove (dialog);
  gtk_widget_destroy (dialog);
}

static void
key_press_cb (GtkWidget * widget, GdkEventKey * event, gpointer data)
{
  if (event->keyval == GDK_Escape)
    cancel_cb (NULL, widget);
}

void
create_init_filetype_dialog (gchar * ext, GtkWidget * clist)
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *table;

  init_filetype_dialog = gtk_dialog_new ();
  dialog_vbox = GTK_DIALOG (init_filetype_dialog)->vbox;
  action_area = GTK_DIALOG (init_filetype_dialog)->action_area;
  gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox), 5);
  gtk_container_set_border_width (GTK_CONTAINER (action_area), 5);
  gtk_signal_connect (GTK_OBJECT (init_filetype_dialog), "key_press_event",
		      GTK_SIGNAL_FUNC (key_press_cb), NULL);

  add_label (dialog_vbox, "Please enter information for this filetype:",
	     0.5, FALSE, 2);

  table = add_table (dialog_vbox, 5, 2, FALSE, FALSE, 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  add_label_to_table (table, "Description: ", 0.9, 0, 1, 0, 1);
  description_entry = add_entry_to_table (table, "", 1, 2, 0, 1);
  gtk_widget_grab_focus (description_entry);

  add_label_to_table (table, "Extension(s): ", 0.9, 0, 1, 1, 2);
  extensions_entry = add_entry_to_table (table, ext, 1, 2, 1, 2);

  add_label_to_table (table, "Format: (mpg,mpeg,mpe)", 1, 1, 2, 2, 3);

  add_label_to_table (table, "Program to open with: ", 0.8, 0, 1, 4, 5);
  program_entry = add_entry_to_table (table, "", 1, 2, 4, 5);

  table = add_table (action_area, 2, 2, TRUE, TRUE, 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5);

  add_button_to_table (table, "Add this extension to an existing filetype",
		       add_ext_to_filetype_cb, NULL, 0, 2, 0, 1);
  add_button_to_table (table, "Ok", init_filetype_ok_cb, clist, 0, 1, 1, 2);
  add_button_to_table (table, "Cancel", cancel_cb, init_filetype_dialog,
		       1, 2, 1, 2);

  gtk_window_set_position (GTK_WINDOW (init_filetype_dialog),
			   GTK_WIN_POS_MOUSE);
  gtk_widget_show (init_filetype_dialog);
  gtk_grab_add (init_filetype_dialog);
  gtk_window_set_transient_for (GTK_WINDOW (init_filetype_dialog),
				GTK_WINDOW (app.main_window));
}
