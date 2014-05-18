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


#include <string.h>
#include "vide.h"

static GtkWidget *dialog;
static GtkWidget *pattern_entry;
static GtkWidget *invert_check;

static void
ok_cb (GtkWidget * widget, FileView * view)
{
  gchar *s;

  s = gtk_entry_get_text (GTK_ENTRY (pattern_entry));
  if (strlen (s) == 0)
    {
      gtk_grab_remove (dialog);
      gtk_widget_destroy (dialog);
			if(1 < gtk_main_level())
				gtk_main_quit();
      return;
    }

  view->filename_filter.active = TRUE;
  /*set_filter_menu_active (view); */

  strncpy (view->filename_filter.pattern, s,
	   sizeof (view->filename_filter.pattern));
  view->filename_filter.invert_mask =
    GTK_TOGGLE_BUTTON (invert_check)->active;


  gtk_grab_remove (dialog);
  gtk_widget_destroy (dialog);
	if(1 < gtk_main_level())
		gtk_main_quit();

  load_dir_list (view);
}

static void
cancel_cb (GtkWidget * widget)
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

void
create_filename_filter_dialog (FileView * view)
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkTooltips *tooltips;

  dialog = gtk_dialog_new ();
  dialog_vbox = GTK_DIALOG (dialog)->vbox;
  action_area = GTK_DIALOG (dialog)->action_area;
  gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox), 5);
  gtk_box_set_spacing (GTK_BOX (dialog_vbox), 5);
  gtk_signal_connect (GTK_OBJECT (dialog), "key_press_event",
		      GTK_SIGNAL_FUNC (key_press_cb), NULL);

  tooltips = gtk_tooltips_new ();

  add_label (dialog_vbox, "Filename Filter: ", 0.0, FALSE, 5);
  pattern_entry = add_entry (dialog_vbox, view->filename_filter.pattern,
			     FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (pattern_entry), "activate",
		      GTK_SIGNAL_FUNC (ok_cb), view);
  gtk_widget_grab_focus (pattern_entry);

  add_label (dialog_vbox, "Example: \\.c$  ^file", 0.0, FALSE, 0);

  invert_check = add_check_button (dialog_vbox, "Invert",
				   view->filename_filter.invert_mask,
				   TRUE, 0, NULL, NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), invert_check,
			"Show files that DO NOT match the given mask", NULL);

  add_button (action_area, "Ok", TRUE, 0, ok_cb, view);
  add_button (action_area, "Cancel", TRUE, 0, cancel_cb, NULL);

  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_widget_show (dialog);
  gtk_grab_add (dialog);
  gtk_window_set_transient_for (GTK_WINDOW (dialog),
				GTK_WINDOW (app.main_window));
}
