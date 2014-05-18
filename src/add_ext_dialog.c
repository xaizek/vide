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


static GtkWidget *add_ext_dialog;
static GtkWidget *filetypes_clist;
static gchar *new_extension;

static void
add_ext_ok_cb (GtkWidget * widget)
{
  FileType *ft;
  gchar *program, *s;
  gint selected_row;

  if (GTK_CLIST (filetypes_clist)->selection == NULL)
    {
      status_bar_message("Please select a filetype first\n");
      return;
    }
  else
    {
      selected_row = (gint) GTK_CLIST (filetypes_clist)->selection->data;
    }

  ft = gtk_clist_get_row_data (GTK_CLIST (filetypes_clist), selected_row);
  strncat (ft->extensions, ",", sizeof (ft->extensions));
  strncat (ft->extensions, new_extension, sizeof (ft->extensions));
  write_filetypes_file ();

  program = s = strdup (ft->programs);
  if ((s = strchr (program, ',')) != NULL)
    *s = '\0';

  exec_action (program);

  free (program);
  free (new_extension);
  gtk_grab_remove (add_ext_dialog);
  gtk_widget_destroy (add_ext_dialog);
	if(1 < gtk_main_level())
		gtk_main_quit();
}

static void
filetypes_select_row_cb (GtkWidget * clist,
			 gint row, gint col, GdkEvent * event, gpointer label)
{
  gchar label_text[NAME_MAX];
  gchar *program, *s;
  FileType *ft = gtk_clist_get_row_data (GTK_CLIST (clist), row);

  if (event->type == GDK_2BUTTON_PRESS)
    {
      add_ext_ok_cb (NULL);
      return;
    }

  program = s = strdup (ft->programs);
  if ((s = strchr (program, ',')) != NULL)
    *s = '\0';

  g_snprintf (label_text, sizeof (label_text), "Default Action: %s", program);
  gtk_label_set_text (GTK_LABEL (GTK_WIDGET (label)), label_text);
  free (program);
}

static void
cancel_cb (GtkWidget * widget)
{
  free (new_extension);
  gtk_grab_remove (add_ext_dialog);
  gtk_widget_destroy (add_ext_dialog);
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
create_add_ext_dialog (gchar * ext)
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *sw;
  GtkWidget *default_program_label;
  GList *tmp;
  gchar *filetypes_titles[2] = { "Extensions", "Description" };
  gchar *buf[2];
  gchar label_text[MAX_LEN];

  if (cfg.filetypes == NULL)
    {
      status_bar_message("No Filetypes exist\n");
      return;
    }

  new_extension = strdup (ext);

  add_ext_dialog = gtk_dialog_new ();
  gtk_widget_set_usize (add_ext_dialog, 250, 275);
  dialog_vbox = GTK_DIALOG (add_ext_dialog)->vbox;
  action_area = GTK_DIALOG (add_ext_dialog)->action_area;
  gtk_signal_connect (GTK_OBJECT (add_ext_dialog), "key_press_event",
		      GTK_SIGNAL_FUNC (key_press_cb), NULL);

  gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox), 5);

  g_snprintf (label_text, sizeof (label_text),
	      "Choose filetype for extension: %s", ext);
  add_label (dialog_vbox, label_text, 0, FALSE, 5);

  sw =
    add_sw (dialog_vbox, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC, TRUE, 5);

  filetypes_clist = gtk_clist_new_with_titles (2, filetypes_titles);
  gtk_container_add (GTK_CONTAINER (sw), filetypes_clist);
  gtk_widget_show (filetypes_clist);

  for (tmp = cfg.filetypes; tmp != NULL; tmp = tmp->next)
    {
      FileType *ft = tmp->data;
      gint i;

      buf[0] = ft->extensions;
      buf[1] = ft->description;

      i = gtk_clist_append (GTK_CLIST (filetypes_clist), buf);
      gtk_clist_set_row_data (GTK_CLIST (filetypes_clist), i, ft);
    }
	g_list_free(tmp);

  default_program_label = add_label (dialog_vbox, "Default Action: ",
				     0.01, FALSE, 5);
  gtk_signal_connect (GTK_OBJECT (filetypes_clist), "select_row",
		      GTK_SIGNAL_FUNC (filetypes_select_row_cb),
		      default_program_label);

  add_button (action_area, "Ok", TRUE, 5, add_ext_ok_cb, NULL);
  add_button (action_area, "Cancel", TRUE, 5, cancel_cb, NULL);

  gtk_window_set_position (GTK_WINDOW (add_ext_dialog), GTK_WIN_POS_CENTER);
  gtk_widget_show (add_ext_dialog);
  gtk_grab_add (add_ext_dialog);
  gtk_window_set_transient_for (GTK_WINDOW (add_ext_dialog),
				GTK_WINDOW (app.main_window));
}
