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

static void
confirm_cb (GtkWidget * widget, guint * answer)
{
  *answer = GPOINTER_TO_UINT (gtk_object_get_user_data (GTK_OBJECT (widget)));

  gtk_grab_remove (dialog);
  gtk_widget_destroy (dialog);
  gtk_main_quit ();
}
static void
error_cb(GtkWidget *widget, gpointer data)
{
	gtk_grab_remove(dialog);
	gtk_widget_destroy(dialog);
	if(1 < gtk_main_level())
		gtk_main_quit();
}

static void
delete_event_cb (GtkWidget * widget)
{
  /* this is just here so the user can't close the dialog without clicking
   * one of the buttons
   */
}

static void
key_press_cb (GtkWidget * widget, GdkEventKey * event, guint * answer)
{
  if (event->keyval == GDK_Escape)
    {
      *answer = CANCEL;
      gtk_grab_remove (dialog);
      gtk_widget_destroy (dialog);
      gtk_main_quit ();
    }
}

void
create_error_dialog(gchar *label_text)
{
	GtkWidget *button;
	GtkWidget *dialog_vbox;
	GtkWidget *action_area;

  dialog = gtk_dialog_new ();
  dialog_vbox = GTK_DIALOG (dialog)->vbox;
  action_area = GTK_DIALOG (dialog)->action_area;
  gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox), 5);
  gtk_signal_connect (GTK_OBJECT (dialog), "delete_event",
		      GTK_SIGNAL_FUNC (delete_event_cb), NULL);
  gtk_signal_connect (GTK_OBJECT (dialog), "key_press_event",
		      GTK_SIGNAL_FUNC (key_press_cb), NULL);
	add_label(dialog_vbox, "Unable to execute command", 0, TRUE, 5);
  add_label (dialog_vbox, label_text, 0, TRUE, 5);

  button = add_button (action_area, "OK", TRUE, 0, error_cb, NULL);
  gtk_widget_grab_focus (button);

  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_widget_show (dialog);
  gtk_grab_add (dialog);
  gtk_window_set_transient_for (GTK_WINDOW (dialog),
				GTK_WINDOW (app.main_window));
}
void
create_confirm_dialog (gchar * label_text, guint * answer)
{
  GtkWidget *button;
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;

  dialog = gtk_dialog_new ();
  dialog_vbox = GTK_DIALOG (dialog)->vbox;
  action_area = GTK_DIALOG (dialog)->action_area;
  gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox), 5);
  gtk_signal_connect (GTK_OBJECT (dialog), "delete_event",
		      GTK_SIGNAL_FUNC (delete_event_cb), NULL);
  gtk_signal_connect (GTK_OBJECT (dialog), "key_press_event",
		      GTK_SIGNAL_FUNC (key_press_cb), answer);

  add_label (dialog_vbox, label_text, 0, TRUE, 5);

  button = add_button (action_area, "Yes", TRUE, 0, confirm_cb, answer);
  gtk_object_set_user_data (GTK_OBJECT (button), GUINT_TO_POINTER (YES));
  gtk_widget_grab_focus (button);

  button =
    add_button (action_area, "Yes To All", TRUE, 0, confirm_cb, answer);
  gtk_object_set_user_data (GTK_OBJECT (button),
			    GUINT_TO_POINTER (YES_TO_ALL));
  button = add_button (action_area, "No", TRUE, 0, confirm_cb, answer);
  gtk_object_set_user_data (GTK_OBJECT (button), GUINT_TO_POINTER (NO));

  button = add_button (action_area, "Cancel", TRUE, 0, confirm_cb, answer);
  gtk_object_set_user_data (GTK_OBJECT (button), GUINT_TO_POINTER (CANCEL));

  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_widget_show (dialog);
  gtk_grab_add (dialog);
  gtk_window_set_transient_for (GTK_WINDOW (dialog),
				GTK_WINDOW (app.main_window));
}
