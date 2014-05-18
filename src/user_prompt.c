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


#include "vide.h"

static GtkWidget *dialog;
static GtkWidget *entry;

static void
ok_cb (GtkWidget * widget, gchar ** string)
{
  gchar *s;

  s = gtk_entry_get_text (GTK_ENTRY (entry));
  if (strlen (s) == 0)
    {
      status_bar_message("Invalid input\n");
      return;
    }

  *string = g_strdup (s);

  gtk_grab_remove (dialog);
  gtk_widget_destroy (dialog);
  gtk_main_quit ();
}

static void
cancel_cb (GtkWidget * widget, gchar ** string)
{
  *string = NULL;
  gtk_grab_remove (dialog);
  gtk_widget_destroy (dialog);
  gtk_main_quit ();
}

static void
key_press_cb (GtkWidget * widget, GdkEventKey * event, gchar ** string)
{
  if (event->keyval == GDK_Escape)
    cancel_cb (NULL, string);
}

static void
delete_event_cb (GtkWidget * widget)
{
  /* this is just here so the user can't close the dialog without clicking
   * the Ok button
   */
}

void
create_user_prompt (gchar * prompt, gchar * init_text, gchar ** string)
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;

  dialog = gtk_dialog_new ();
  dialog_vbox = GTK_DIALOG (dialog)->vbox;
  action_area = GTK_DIALOG (dialog)->action_area;
  gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox), 5);
  gtk_box_set_spacing (GTK_BOX (dialog_vbox), 5);
  gtk_signal_connect (GTK_OBJECT (dialog), "delete_event", delete_event_cb,
		      NULL);
  gtk_signal_connect (GTK_OBJECT (dialog), "key_press_event",
		      GTK_SIGNAL_FUNC (key_press_cb), string);

  add_label (dialog_vbox, prompt, 0.0, FALSE, 5);
  entry = add_entry (dialog_vbox, init_text, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (entry), "activate",
		      GTK_SIGNAL_FUNC (ok_cb), string);
  gtk_widget_grab_focus (entry);

  add_button (action_area, "Ok", TRUE, 0, ok_cb, string);
  add_button (action_area, "Cancel", TRUE, 0, cancel_cb, string);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_widget_show (dialog);
  gtk_grab_add (dialog);
  gtk_window_set_transient_for (GTK_WINDOW (dialog),
				GTK_WINDOW (app.main_window));
}
