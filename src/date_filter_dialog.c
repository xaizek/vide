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
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static GtkWidget *dialog;
static GtkWidget *date_entry;
static GtkWidget *operation_combo;

void
ok_cb (GtkWidget * widget, FileView * view)
{
  gchar date_string[MAX_LEN];
  gchar *s;
  struct tm tm_time;

  view->date_filter.active = TRUE;
 /* set_filter_menu_active (view); */

  g_snprintf (date_string, sizeof (date_string),
	      "%s 00:00:00", gtk_entry_get_text (GTK_ENTRY (date_entry)));
  strptime (date_string, "%m-%d-%Y %T", &tm_time);
  view->date_filter.time = mktime (&tm_time);

  s = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (operation_combo)->entry));
  if (STREQ (s, "Accessed Since"))
    {
      view->date_filter.op = GT;
      view->date_filter.time_type = ATIME;
    }
  else if (STREQ (s, "Accessed Before"))
    {
      view->date_filter.op = LT;
      view->date_filter.time_type = ATIME;
    }
  else if (STREQ (s, "Modified Since"))
    {
      view->date_filter.op = GT;
      view->date_filter.time_type = MTIME;
    }
  else if (STREQ (s, "Modified Before"))
    {
      view->date_filter.op = LT;
      view->date_filter.time_type = MTIME;
    }
  else if (STREQ (s, "Changed Since"))
    {
      view->date_filter.op = GT;
      view->date_filter.time_type = CTIME;
    }
  else if (STREQ (s, "Changed Before"))
    {
      view->date_filter.op = LT;
      view->date_filter.time_type = CTIME;
    }

  gtk_grab_remove (dialog);
  gtk_widget_destroy (dialog);
	if(1 < gtk_main_level())
		gtk_main_quit();
  load_dir_list (view);
}

void
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
create_date_filter_dialog (FileView * view)
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *hbox;
  gchar date_string[11];
  struct tm *tm_ptr;
  GList *tmp = NULL;

  dialog = gtk_dialog_new ();
  dialog_vbox = GTK_DIALOG (dialog)->vbox;
  action_area = GTK_DIALOG (dialog)->action_area;
  gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox), 5);
  gtk_box_set_spacing (GTK_BOX (dialog_vbox), 5);
  gtk_signal_connect (GTK_OBJECT (dialog), "key_press_event",
		      GTK_SIGNAL_FUNC (key_press_cb), NULL);

  add_label (dialog_vbox, "Date Filter: ", 0.0, FALSE, 5);

  hbox = add_hbox (dialog_vbox, FALSE, 5, TRUE, 5);

  operation_combo = gtk_combo_new ();
  gtk_widget_set_usize (operation_combo, 130, 0);
  gtk_box_pack_start (GTK_BOX (hbox), operation_combo, FALSE, FALSE, 0);
  gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (operation_combo)->entry),
			  FALSE);
  gtk_widget_show (operation_combo);

  tmp = g_list_append (tmp, strdup ("Accessed Since"));
  tmp = g_list_append (tmp, strdup ("Accessed Before"));
  tmp = g_list_append (tmp, strdup ("Modified Since"));
  tmp = g_list_append (tmp, strdup ("Modified Before"));
  tmp = g_list_append (tmp, strdup ("Changed Since"));
  tmp = g_list_append (tmp, strdup ("Changed Before"));
  gtk_combo_set_popdown_strings (GTK_COMBO (operation_combo), tmp);
  switch (view->date_filter.time_type)
    {
    case ATIME:
      if (view->date_filter.op == GT)
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (operation_combo)->entry),
			    "Accessed Since");
      else
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (operation_combo)->entry),
			    "Accessed Before");
      break;
    case MTIME:
      if (view->date_filter.op == GT)
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (operation_combo)->entry),
			    "Modified Since");
      else
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (operation_combo)->entry),
			    "Modified Before");
      break;
    case CTIME:
      if (view->date_filter.op == GT)
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (operation_combo)->entry),
			    "Changed Since");
      else
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (operation_combo)->entry),
			    "Changed Before");
      break;
    default:
      gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (operation_combo)->entry),
			  "Accessed Since");
      break;
    }

  tm_ptr = localtime (&(view->date_filter.time));
  strftime (date_string, sizeof (date_string), "%m-%d-%Y", tm_ptr);
  date_entry = add_entry (hbox, date_string, FALSE, 0);
  gtk_widget_set_usize (date_entry, 100, 0);

  add_button (action_area, "Ok", FALSE, 0, ok_cb, view);
  add_button (action_area, "Cancel", FALSE, 0, cancel_cb, NULL);

  gtk_widget_show (dialog);
  gtk_grab_add (dialog);
  gtk_window_set_transient_for (GTK_WINDOW (dialog),
				GTK_WINDOW (app.main_window));
	g_list_free(tmp);
}
