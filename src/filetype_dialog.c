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

static GtkWidget *filetype_dialog;
static GtkWidget *actions_list;
static GtkWidget *description_entry;
static GtkWidget *extensions_entry;
static FileType *this_filetype;


static void
add_action_cb (GtkWidget * widget)
{
  gchar label[NAME_MAX];
  gchar *extensions = gtk_entry_get_text (GTK_ENTRY (extensions_entry));
  gchar *description = gtk_entry_get_text (GTK_ENTRY (description_entry));
  gchar *new_action;

  if (STREQ (extensions, ""))
    {
      status_bar_message("Extensions field cannot be blank\n");
      return;
    }
  g_snprintf (label, sizeof (label), "Enter program for %s", description);

  create_user_prompt (label, "", &new_action);
  gtk_main ();

  if (new_action != NULL)
    {
      gchar *buf[1];

      buf[0] = new_action;
      gtk_clist_append (GTK_CLIST (actions_list), buf);

      g_free (new_action);
    }
}

static void
change_action_cb (GtkWidget * widget)
{
  gint selected_row;
  gchar *action;
  gchar *new_action;

  if (GTK_CLIST (actions_list)->selection != NULL)
    selected_row = (gint) GTK_CLIST (actions_list)->selection->data;
  else
    return;

  if (selected_row < 0)
    return;

  gtk_clist_get_text (GTK_CLIST (actions_list), selected_row, 0, &action);

  create_user_prompt ("Enter new action:", action, &new_action);
  gtk_main ();

  if (new_action != NULL)
    {
      gtk_clist_set_text (GTK_CLIST (actions_list), selected_row, 0,
			  new_action);
      g_free (new_action);
    }
}

static void
remove_action_cb (GtkWidget * widget)
{
  gint selected_row;

  if (GTK_CLIST (actions_list)->selection != NULL)
    selected_row = (gint) GTK_CLIST (actions_list)->selection->data;
  else
    return;

  if (selected_row < 0)
    return;

  gtk_clist_remove (GTK_CLIST (actions_list), selected_row);
}

static void
set_default_action_cb (GtkWidget * widget)
{
  gint selected_row;

  if (GTK_CLIST (actions_list)->selection != NULL)
    selected_row = (gint) GTK_CLIST (actions_list)->selection->data;
  else
    return;

  if (selected_row == 0)
    return;

  gtk_clist_swap_rows (GTK_CLIST (actions_list), selected_row, 0);
}

static void
done_cb (GtkWidget * widget, gpointer write_file)
{
  gchar *description = gtk_entry_get_text (GTK_ENTRY (description_entry));
  gchar *extensions = gtk_entry_get_text (GTK_ENTRY (extensions_entry));
  gchar *program;
  gint i;

  strncpy (this_filetype->description, description,
	   sizeof (this_filetype->description));
  strncpy (this_filetype->extensions, extensions,
	   sizeof (this_filetype->extensions));
  strncpy (this_filetype->programs, "", sizeof (this_filetype->programs));

  for (i = 0; i < GTK_CLIST (actions_list)->rows; i++)
    {
      gtk_clist_get_text (GTK_CLIST (actions_list), i, 0, &program);
      strncat (this_filetype->programs, program,
	       sizeof (this_filetype->programs));

      if ((i + 1) != GTK_CLIST (actions_list)->rows)	/* not after the last one */
	strncat (this_filetype->programs, ",",
		 sizeof (this_filetype->programs));
    }

  if (GPOINTER_TO_INT (write_file))
    write_filetypes_file ();

  gtk_grab_remove (filetype_dialog);
  gtk_widget_destroy (filetype_dialog);
	if(1 < gtk_main_level())
		gtk_main_quit();
}

static void
key_press_cb (GtkWidget * widget, GdkEventKey * event, gpointer data)
{
  if (event->keyval == GDK_Escape)
    done_cb (NULL, GINT_TO_POINTER (FALSE));
}

void
create_filetype_dialog (FileType * ft, gboolean write_file)
{
  GtkWidget *sw;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *table;
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  gchar *program, *s, *free_this;
  gchar *buf[1];

  this_filetype = ft;

  filetype_dialog = gtk_dialog_new ();
  gtk_widget_set_usize (filetype_dialog, 250, 250);
  dialog_vbox = GTK_DIALOG (filetype_dialog)->vbox;
  action_area = GTK_DIALOG (filetype_dialog)->action_area;
  gtk_signal_connect (GTK_OBJECT (filetype_dialog), "key_press_event",
		      GTK_SIGNAL_FUNC (key_press_cb), NULL);

  gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox), 5);
  gtk_box_set_spacing (GTK_BOX (dialog_vbox), 5);
  gtk_container_set_border_width (GTK_CONTAINER (action_area), 5);

  table = add_table (dialog_vbox, 2, 2, FALSE, TRUE, 2);
  add_label_to_table (table, "Description:", 0.8, 0, 1, 0, 1);
  description_entry = add_entry_to_table (table, this_filetype->description,
					  1, 2, 0, 1);

  add_label_to_table (table, "Extension(s):", 0.8, 0, 1, 1, 2);
  extensions_entry = add_entry_to_table (table, this_filetype->extensions,
					 1, 2, 1, 2);

  add_label (dialog_vbox, "Actions:", 0.01, FALSE, 0);

  sw = add_sw (dialog_vbox, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC, TRUE, 0);

  actions_list = gtk_clist_new (1);
  gtk_clist_set_selection_mode (GTK_CLIST (actions_list),
				GTK_SELECTION_BROWSE);
  gtk_clist_set_reorderable (GTK_CLIST (actions_list), TRUE);
  gtk_container_add (GTK_CONTAINER (sw), actions_list);
  gtk_widget_show (actions_list);

  if ((s = strdup (this_filetype->programs)) != NULL)
    {
      free_this = program = s;
      while ((s = strchr (program, ',')) != NULL)
	{
	  *s++ = '\0';
	  buf[0] = program;
	  gtk_clist_append (GTK_CLIST (actions_list), buf);
	  program = s;
	}
      buf[0] = program;
      gtk_clist_append (GTK_CLIST (actions_list), buf);
      free (free_this);
    }

  vbox = add_vbox (action_area, TRUE, 2, TRUE, 0);
  hbox = add_hbox (vbox, FALSE, 5, TRUE, 0);
  add_button (hbox, "Add", TRUE, 0, add_action_cb, NULL);
  add_button (hbox, "Remove", TRUE, 0, remove_action_cb, NULL);
  add_button (hbox, "Change", TRUE, 0, change_action_cb, NULL);
  add_button (hbox, "Set Default", TRUE, 0, set_default_action_cb, NULL);

  hbox = add_hbox (vbox, TRUE, 2, TRUE, 2);
  add_button (hbox, "Done", TRUE, 0, done_cb, GINT_TO_POINTER (write_file));

  gtk_window_set_position (GTK_WINDOW (filetype_dialog), GTK_WIN_POS_MOUSE);
  gtk_widget_show (filetype_dialog);
  gtk_grab_add (filetype_dialog);
  gtk_window_set_transient_for (GTK_WINDOW (filetype_dialog),
				GTK_WINDOW (app.main_window));
}
