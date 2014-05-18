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


#include <unistd.h>
#include "vide.h"

static GtkWidget *dialog;
static GtkWidget *entry;

static void
ok_cb (GtkWidget * widget, gchar * filename)
{
  gchar *new_name;
  //gint overwrite_button;

  if ((new_name = gtk_entry_get_text (GTK_ENTRY (entry))) == NULL)
    return;

    file_copy (filename, new_name);

  	refresh_list (curr_view);
  	gtk_grab_remove (dialog);
  	gtk_widget_destroy (dialog);
}

static void
cancel_cb (GtkWidget * widget)
{
  gtk_grab_remove (dialog);
  gtk_widget_destroy (dialog);
}

static void
key_press_cb (GtkWidget * widget, GdkEventKey * event, gpointer data)
{
  if (event->keyval == GDK_Escape)
    cancel_cb (NULL);
}

void
create_copy_here_dialog ()
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *wid;
  FileInfo *info;

  dialog = gtk_dialog_new ();
  dialog_vbox = GTK_DIALOG (dialog)->vbox;
  action_area = GTK_DIALOG (dialog)->action_area;
  gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox), 5);
  gtk_box_set_spacing (GTK_BOX (dialog_vbox), 5);
  gtk_signal_connect (GTK_OBJECT (dialog), "key_press_event",
		      GTK_SIGNAL_FUNC (key_press_cb), NULL);

  wid = add_label (dialog_vbox, "This allows you to copy a file\n"
		   "to the current directory\n"
		   "rather than the opposite directory.\n\n"
		   "Enter New Filename: ", 0.0, FALSE, 5);
  gtk_label_set_justify (GTK_LABEL (wid), GTK_JUSTIFY_LEFT);

  info =
    gtk_clist_get_row_data (GTK_CLIST (curr_view->clist), curr_view->row);
  entry = add_entry (dialog_vbox, info->filename, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (entry), "activate",
		      GTK_SIGNAL_FUNC (ok_cb), info->filename);
  gtk_widget_grab_focus (entry);

  add_button (action_area, "Ok", TRUE, 0, ok_cb, info->filename);
  add_button (action_area, "Cancel", TRUE, 0, cancel_cb, NULL);

  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_widget_show (dialog);
  gtk_grab_add (dialog);
  gtk_window_set_transient_for (GTK_WINDOW (dialog),
				GTK_WINDOW (app.main_window));
}
