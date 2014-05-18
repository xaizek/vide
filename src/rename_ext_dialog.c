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
#include <string.h>
#include "vide.h"

static GtkWidget *dialog;
static GtkWidget *from_entry;
static GtkWidget *to_entry;

static void
ok_cb (GtkWidget * widget)
{
  gchar new_name[NAME_MAX];
  gchar *from_sub = gtk_entry_get_text (GTK_ENTRY (from_entry));
  gchar *to_sub = gtk_entry_get_text (GTK_ENTRY (to_entry));
  gint i;

  if (STREQ (from_sub, "") || STREQ (to_sub, ""))
    return;

  for (i = 0; i < GTK_CLIST (curr_view->clist)->rows; i++)
    {
      FileInfo *info =
	gtk_clist_get_row_data (GTK_CLIST (curr_view->clist), i);
      gchar *dup, *s;

      dup = strdup (info->filename);
      if ((s = strstr (dup, from_sub)) != NULL)
	{
	  gchar *pre = dup;
	  *s = '\0';
	  s += strlen (from_sub);
	  g_snprintf (new_name, sizeof (new_name), "%s%s%s", pre, to_sub, s);

	  file_move (info->filename, new_name);
	}
      g_free (dup);
    }

  gtk_grab_remove (dialog);
  gtk_widget_destroy (dialog);
  refresh_list (curr_view);
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
create_rename_ext_dialog ()
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *table;
  GtkWidget *wid;

  dialog = gtk_dialog_new ();
  dialog_vbox = GTK_DIALOG (dialog)->vbox;
  action_area = GTK_DIALOG (dialog)->action_area;
  gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox), 5);
  gtk_box_set_spacing (GTK_BOX (dialog_vbox), 5);
  gtk_signal_connect (GTK_OBJECT (dialog), "key_press_event",
		      GTK_SIGNAL_FUNC (key_press_cb), NULL);

  wid = add_label (dialog_vbox, "This allows you to replace substrings\n"
		   "in filenames with new strings.\n\n"
		   "Example: (From: .jpg  To: .jpeg)", 0.0, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (wid), GTK_JUSTIFY_LEFT);

  table = add_table (dialog_vbox, 2, 2, FALSE, FALSE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);

  add_label_to_table (table, "From: ", 0.0, 0, 1, 0, 1);
  from_entry = add_entry_to_table (table, "", 1, 2, 0, 1);
  gtk_widget_grab_focus (from_entry);
  gtk_signal_connect (GTK_OBJECT (from_entry), "activate",
		      GTK_SIGNAL_FUNC (ok_cb), NULL);

  add_label_to_table (table, "To: ", 0.0, 0, 1, 1, 2);
  to_entry = add_entry_to_table (table, "", 1, 2, 1, 2);
  gtk_signal_connect (GTK_OBJECT (to_entry), "activate",
		      GTK_SIGNAL_FUNC (ok_cb), NULL);

  add_button (action_area, "Ok", TRUE, 0, ok_cb, NULL);
  add_button (action_area, "Cancel", TRUE, 0, cancel_cb, NULL);

  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);
  gtk_widget_show (dialog);
  gtk_grab_add (dialog);
  gtk_window_set_transient_for (GTK_WINDOW (dialog),
				GTK_WINDOW (app.main_window));
}
