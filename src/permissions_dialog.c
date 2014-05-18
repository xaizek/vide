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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>
#include "vide.h"

static GtkWidget *permissions_dialog;
static mode_t mask[12] =
  { S_ISUID, S_ISGID, S_ISVTX, S_IRUSR, S_IWUSR, S_IXUSR,
  S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH
};
static gchar *add_strings[12] = { "u+s", "g+s", "o+t", "u+r", "u+w", "u+x",
  "g+r", "g+w", "g+x", "o+r", "o+w", "o+x"
};
static gchar *remove_strings[12] = { "u-s", "g-s", "o-t", "u-r", "u-w", "u-x",
  "g-r", "g-w", "g-x", "o-r", "o-w", "o-x"
};

enum
{ SETUID, SETGID, STICKY, USR_READ, USR_WRITE, USR_EXEC, GRP_READ,
  GRP_WRITE, GRP_EXEC, OTH_READ, OTH_WRITE, OTH_EXEC
};
static GtkWidget *chmod_buttons[12];
static GtkWidget *set_perms_button;
static GtkWidget *add_perms_button;
static GtkWidget *remove_perms_button;
static GtkWidget *recurse_dirs_button;

static void
get_mode_string (gchar * mode_string, gint length)
{
  int i;

  if (GTK_TOGGLE_BUTTON (set_perms_button)->active)
    {
      mode_t mode = 0;
      for (i = 0; i < 12; i++)
	{
	  if (GTK_TOGGLE_BUTTON (chmod_buttons[i])->active)
	    mode |= mask[i];
	}
      g_snprintf (mode_string, length, "%o", (unsigned int) mode);
      return;
    }
  else if (GTK_TOGGLE_BUTTON (add_perms_button)->active)
    {
      *mode_string = '\0';
      for (i = 0; i < 12; i++)
	{
	  if (GTK_TOGGLE_BUTTON (chmod_buttons[i])->active)
	    {
	      strncat (mode_string, add_strings[i], length);
	      strncat (mode_string, ",", length);
	    }
	}
      i = strlen (mode_string);
      if (i > 0)
	mode_string[i - 1] = '\0';	/* get rid of trailing comma */
      return;
    }
  else if (GTK_TOGGLE_BUTTON (remove_perms_button)->active)
    {
      *mode_string = '\0';
      for (i = 0; i < 12; i++)
	{
	  if (GTK_TOGGLE_BUTTON (chmod_buttons[i])->active)
	    {
	      strncat (mode_string, remove_strings[i], length);
	      strncat (mode_string, ",", length);
	    }
	}
      i = strlen (mode_string);
      if (i > 0)
	mode_string[i - 1] = '\0';
      return;
    }
  else
    {
      fprintf (stderr, "Something messed up. Exit quickly.\n");
    }

  return;
}

static void
ok_cb (GtkWidget * widget)
{
  FileInfo *info;
  gchar path[PATH_MAX + NAME_MAX];
  gchar mode_string[50];

  get_mode_string (mode_string, sizeof (mode_string));
  info = curr_view->iter->data;
  g_snprintf (path, sizeof (path), "%s/%s", curr_view->dir, info->filename);
  file_chmod (path, mode_string,
	      GTK_TOGGLE_BUTTON (recurse_dirs_button)->active);

  gtk_grab_remove (permissions_dialog);
  gtk_widget_destroy (permissions_dialog);

  curr_view->iter = curr_view->iter->next;
  if (curr_view->iter != NULL)
    {
      info = curr_view->iter->data;
      create_permissions_dialog (info);
    }
  else
    {
      g_list_free (curr_view->iter_base);
      refresh_list (curr_view);
      gtk_clist_select_row (GTK_CLIST (curr_view->clist), curr_view->row, 0);
      gtk_widget_grab_focus (curr_view->clist);
    }
}

static void
apply_to_all_cb (GtkWidget * widget)
{
  FileInfo *info;
  gchar path[PATH_MAX + NAME_MAX];
  gchar mode_string[50];

  get_mode_string (mode_string, sizeof (mode_string));

  for (; curr_view->iter != NULL; curr_view->iter = curr_view->iter->next)
    {
      info = curr_view->iter->data;
      g_snprintf (path, sizeof (path), "%s/%s", curr_view->dir,
		  info->filename);

      file_chmod (path, mode_string,
		  GTK_TOGGLE_BUTTON (recurse_dirs_button)->active);
    }

  g_list_free (curr_view->iter_base);
  gtk_grab_remove (permissions_dialog);
  gtk_widget_destroy (permissions_dialog);
  refresh_list (curr_view);
  gtk_clist_select_row (GTK_CLIST (curr_view->clist), curr_view->row, 0);
  gtk_widget_grab_focus (curr_view->clist);
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

static void
add_chmod_button (GtkWidget * box, gint i, gchar * label, gint state)
{
  chmod_buttons[i] = gtk_toggle_button_new_with_label (label);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chmod_buttons[i]), state);
  gtk_box_pack_start (GTK_BOX (box), chmod_buttons[i], TRUE, TRUE, 5);
  gtk_widget_show (chmod_buttons[i]);
}

static void
clear_chmod_buttons (GtkWidget * widget)
{
  int i;

  for (i = 0; i < 12; i++)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chmod_buttons[i]),
				  FALSE);
}

static void
reset_chmod_buttons (GtkWidget * widget, FileInfo * info)
{
  int i;

  for (i = 0; i < 12; i++)
    {
      if (info->statbuf.st_mode & mask[i])
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chmod_buttons[i]),
				      TRUE);
      else
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (chmod_buttons[i]),
				      FALSE);
    }
}


void
create_permissions_dialog (FileInfo * info)
{
  GtkWidget *sub_vbox;
  GtkWidget *hbox;
  GtkWidget *frame;
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *table;
  gchar label_text[NAME_MAX + 20];
  struct passwd *pw_buf;
  struct group *grp_buf;

  permissions_dialog = gtk_dialog_new ();
  dialog_vbox = GTK_DIALOG (permissions_dialog)->vbox;
  action_area = GTK_DIALOG (permissions_dialog)->action_area;
  gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox), 5);
  gtk_signal_connect (GTK_OBJECT (permissions_dialog), "key_press_event",
		      GTK_SIGNAL_FUNC (key_press_cb), NULL);

  g_snprintf (label_text, sizeof (label_text),
	      "Filename: %s", info->filename);
  add_label (dialog_vbox, label_text, 0, TRUE, 5);

  table = add_table (dialog_vbox, 2, 1, TRUE, TRUE, 0);

  if ((pw_buf = getpwuid (info->statbuf.st_uid)) != NULL)
    {
      g_snprintf (label_text, sizeof (label_text),
		  "User: %s", pw_buf->pw_name);
    }
  else
    {
      g_snprintf (label_text, sizeof (label_text),
		  "User: %d", (int) info->statbuf.st_uid);
    }
  add_label_to_table (table, label_text, 0, 0, 1, 0, 1);

  if ((grp_buf = getgrgid (info->statbuf.st_gid)) != NULL)
    {
      g_snprintf (label_text, sizeof (label_text),
		  "Group: %s", grp_buf->gr_name);
    }
  else
    {
      g_snprintf (label_text, sizeof (label_text),
		  "Group: %d", (int) info->statbuf.st_gid);
    }
  add_label_to_table (table, label_text, 0, 0, 1, 1, 2);

  frame = gtk_frame_new ("File Permissions");
  gtk_box_pack_start (GTK_BOX (dialog_vbox), frame, TRUE, TRUE, 5);
  gtk_widget_show (frame);

  sub_vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), sub_vbox);
  gtk_widget_show (sub_vbox);

  hbox = add_hbox (sub_vbox, TRUE, 0, TRUE, 0);
  add_label (hbox, "", 0.5, TRUE, 0);
  add_label (hbox, "Read", 0.5, TRUE, 0);
  add_label (hbox, "Write", 0.5, TRUE, 0);
  add_label (hbox, "Exec", 0.5, TRUE, 0);
  add_label (hbox, "Special", 0.5, TRUE, 0);

  hbox = add_hbox (sub_vbox, TRUE, 0, TRUE, 5);
  add_label (hbox, "User", 0.5, TRUE, 0);
  add_chmod_button (hbox, USR_READ, "", info->statbuf.st_mode & S_IRUSR);
  add_chmod_button (hbox, USR_WRITE, "", info->statbuf.st_mode & S_IWUSR);
  add_chmod_button (hbox, USR_EXEC, "", info->statbuf.st_mode & S_IXUSR);
  add_chmod_button (hbox, SETUID, "Set UID", info->statbuf.st_mode & S_ISUID);

  hbox = add_hbox (sub_vbox, TRUE, 0, TRUE, 5);
  add_label (hbox, "Group", 0.5, TRUE, 0);
  add_chmod_button (hbox, GRP_READ, "", info->statbuf.st_mode & S_IRGRP);
  add_chmod_button (hbox, GRP_WRITE, "", info->statbuf.st_mode & S_IWGRP);
  add_chmod_button (hbox, GRP_EXEC, "", info->statbuf.st_mode & S_IXGRP);
  add_chmod_button (hbox, SETGID, "Set GID", info->statbuf.st_mode & S_ISGID);

  hbox = add_hbox (sub_vbox, TRUE, 0, TRUE, 5);
  add_label (hbox, "Other", 0.5, TRUE, 0);
  add_chmod_button (hbox, OTH_READ, "", info->statbuf.st_mode & S_IROTH);
  add_chmod_button (hbox, OTH_WRITE, "", info->statbuf.st_mode & S_IWOTH);
  add_chmod_button (hbox, OTH_EXEC, "", info->statbuf.st_mode & S_IXOTH);
  add_chmod_button (hbox, STICKY, "Sticky", info->statbuf.st_mode & S_ISVTX);

  frame = gtk_frame_new ("Action:");
  gtk_box_pack_start (GTK_BOX (dialog_vbox), frame, TRUE, TRUE, 5);
  gtk_widget_show (frame);

  sub_vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), sub_vbox);
  gtk_widget_show (sub_vbox);

  hbox = add_hbox (sub_vbox, TRUE, 0, TRUE, 0);
  set_perms_button = add_radio_button (hbox, "Set", NULL, TRUE, 5,
				       reset_chmod_buttons, info);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (set_perms_button), TRUE);
  add_perms_button = add_radio_button (hbox, "Add",
				       gtk_radio_button_group
				       (GTK_RADIO_BUTTON (set_perms_button)),
				       TRUE, 5, clear_chmod_buttons, NULL);
  remove_perms_button =
    add_radio_button (hbox, "Remove",
		      gtk_radio_button_group (GTK_RADIO_BUTTON
					      (add_perms_button)), TRUE, 5,
		      clear_chmod_buttons, NULL);

  recurse_dirs_button = add_check_button (dialog_vbox, "Recurse Directories",
					  FALSE, FALSE, 5, NULL, NULL);

  add_button (action_area, "Ok", TRUE, 0, ok_cb, NULL);
  if (g_list_length (curr_view->iter) > 1)
    add_button (action_area, "Apply To All", TRUE, 0, apply_to_all_cb, NULL);
  add_button (action_area, "Cancel", TRUE, 0, cancel_cb, permissions_dialog);

  gtk_window_set_position (GTK_WINDOW (permissions_dialog),
			   GTK_WIN_POS_CENTER);
  gtk_widget_show (permissions_dialog);
  gtk_grab_add (permissions_dialog);
  gtk_window_set_transient_for (GTK_WINDOW (permissions_dialog),
				GTK_WINDOW (app.main_window));
}
