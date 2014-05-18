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
static GtkWidget *entry;

static GString *
expand (gchar * text, gchar * filename)
{
  GString *command_string = g_string_new ("");
  gchar *s, *free_this, *command_copy;

  command_copy = g_strdup (text);
  free_this = s = command_copy;

  if (strchr (command_copy, '%') == NULL)
    {
      g_string_sprintf (command_string, "%s %s", command_copy, filename);
      g_free (free_this);
      return command_string;
    }

  while ((s = strchr (command_copy, '%')) != NULL)
    {
      *s++ = '\0';
      g_string_append (command_string, command_copy);
      command_copy = s + 1;
      switch (*s)
	{
	case 'f':
	  g_string_sprintfa (command_string, "\"%s\"", filename);
	  break;
	case 'd':
	  g_string_sprintfa (command_string, "\"%s\"", curr_view->dir);
	  break;
	case 'D':
	  g_string_sprintfa (command_string, "\"%s\"", other_view->dir);
	  break;
	default:
	  g_string_append_c (command_string, '%');
	  g_string_append_c (command_string, *s);
	  break;
	}
    }
  g_string_append (command_string, command_copy);
  g_free (free_this);
  return command_string;
}

static void
ok_cb (GtkWidget * widget, gchar * filename)
{
  GList *tmp, *base;
  gchar *text = gtk_entry_get_text (GTK_ENTRY (entry));
  GString *command;

  base = tmp = get_selection (curr_view);
  for (; tmp != NULL; tmp = tmp->next)
    {
      FileInfo *info = tmp->data;
      command = expand (text, info->filename);
      if (strncmp (command->str, "x ", 2) == 0)
	//exec_in_xterm (command->str + 2);
				create_zterm(command->str +2, TRUE);
      else
	file_exec (command->str);
      g_string_free (command, TRUE);
    }
  g_list_free(base);

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
create_for_each_dialog ()
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *wid;

  dialog = gtk_dialog_new ();
  dialog_vbox = GTK_DIALOG (dialog)->vbox;
  action_area = GTK_DIALOG (dialog)->action_area;
  gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox), 5);
  gtk_box_set_spacing (GTK_BOX (dialog_vbox), 5);
  gtk_signal_connect (GTK_OBJECT (dialog), "key_press_event",
		      GTK_SIGNAL_FUNC (key_press_cb), NULL);

  wid = add_label (dialog_vbox, "Enter a command to execute\n"
		   "on each selected file:\n\n"
		   "Macros:\n"
		   "%f = Filename\n"
		   "%d = Active Directory\n"
		   "%D = Inactive Directory", 0.0, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (wid), GTK_JUSTIFY_LEFT);

  entry = add_entry (dialog_vbox, "", FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (entry), "activate",
		      GTK_SIGNAL_FUNC (ok_cb), NULL);
  gtk_widget_grab_focus (entry);

  add_button (action_area, "Ok", TRUE, 0, ok_cb, NULL);
  add_button (action_area, "Cancel", TRUE, 0, cancel_cb, NULL);

  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);
  gtk_widget_show (dialog);
  gtk_grab_add (dialog);
  gtk_window_set_transient_for (GTK_WINDOW (dialog),
				GTK_WINDOW (app.main_window));
}
