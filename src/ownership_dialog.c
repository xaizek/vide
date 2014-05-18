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
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include "vide.h"

static GtkWidget *ownership_dialog;
static GtkWidget *user_combo;
static GtkWidget *group_combo;
static GtkWidget *recurse_dirs_button;

static uid_t
get_user_id ()
{
  gchar *user;
  struct passwd *pw_buf;
  uid_t user_id;

  user = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (user_combo)->entry));
  if ((pw_buf = getpwnam (user)) != NULL)
    user_id = pw_buf->pw_uid;
  else
    user_id = (uid_t) atoi (user);

  return user_id;
}

static gid_t
get_group_id ()
{
  gchar *group;
  struct group *grp_buf;
  gid_t group_id;

  group = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (group_combo)->entry));
  if ((grp_buf = getgrnam (group)) != NULL)
    group_id = grp_buf->gr_gid;
  else
    group_id = (gid_t) atoi (group);

  return group_id;
}

static void
ok_cb (GtkWidget * widget)
{
  FileInfo *info;
  gchar path[PATH_MAX + NAME_MAX];

  info = curr_view->iter->data;
  g_snprintf (path, sizeof (path), "%s/%s", curr_view->dir, info->filename);
  file_chown (path, get_user_id (), get_group_id (),
	      GTK_TOGGLE_BUTTON (recurse_dirs_button)->active);

  gtk_grab_remove (ownership_dialog);
  gtk_widget_destroy (ownership_dialog);

  curr_view->iter = curr_view->iter->next;
  if (curr_view->iter != NULL)
    {
      info = curr_view->iter->data;
      create_ownership_dialog (info);
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

  for (; curr_view->iter != NULL; curr_view->iter = curr_view->iter->next)
    {
      info = curr_view->iter->data;
      g_snprintf (path, sizeof (path), "%s/%s", curr_view->dir,
		  info->filename);

      file_chown (path, get_user_id (), get_group_id (),
		  GTK_TOGGLE_BUTTON (recurse_dirs_button)->active);
    }

  g_list_free (curr_view->iter_base);
  gtk_grab_remove (ownership_dialog);
  gtk_widget_destroy (ownership_dialog);
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

void
create_ownership_dialog (FileInfo * info)
{
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *table;
  gchar label_text[NAME_MAX + 20];
  gchar perm_string[11];

  ownership_dialog = gtk_dialog_new ();
  dialog_vbox = GTK_DIALOG (ownership_dialog)->vbox;
  action_area = GTK_DIALOG (ownership_dialog)->action_area;
  gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox), 5);
  gtk_signal_connect (GTK_OBJECT (ownership_dialog), "key_press_event",
		      GTK_SIGNAL_FUNC (key_press_cb), NULL);

  g_snprintf (label_text, sizeof (label_text),
	      "Filename: %s", info->filename);
  add_label (dialog_vbox, label_text, 0, TRUE, 5);

  get_perm_string (perm_string, sizeof (perm_string), info->statbuf.st_mode);
  g_snprintf (label_text, sizeof (label_text), "Permissions: %s",
	      perm_string);
  add_label (dialog_vbox, label_text, 0, TRUE, 0);

  table = add_table (dialog_vbox, 2, 2, FALSE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table), 5);
  add_label_to_table (table, "User: ", 0, 0, 1, 0, 1);

  user_combo = gtk_combo_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), user_combo, 1, 2, 0, 1);
  gtk_widget_show (user_combo);

  add_label_to_table (table, "Group: ", 0, 0, 1, 1, 2);

  group_combo = gtk_combo_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), group_combo, 1, 2, 1, 2);
  gtk_widget_show (group_combo);

  {
    GList *users = NULL;
    GList *groups = NULL;
    struct passwd *pw_buf;
    struct group *grp_buf;

    if (getuid () == 0)
      {
	while ((pw_buf = getpwent ()) != NULL)
	  users = g_list_append (users, strdup (pw_buf->pw_name));
      }
    else
      {
	pw_buf = getpwuid (getuid ());
	users = g_list_append (users, strdup (pw_buf->pw_name));
      }

    if (getuid () == 0)
      {
	while ((grp_buf = getgrent ()) != NULL)
	  groups = g_list_append (groups, strdup (grp_buf->gr_name));
      }
    else
      {
	gid_t grp_ids[32];
	gint i, n;
	n = getgroups (32, grp_ids);
	for (i = 0; i < n; i++)
	  {
	    grp_buf = getgrgid (grp_ids[i]);
	    groups = g_list_append (groups, strdup (grp_buf->gr_name));
	  }
      }

    gtk_combo_set_popdown_strings (GTK_COMBO (user_combo), users);
    gtk_combo_set_popdown_strings (GTK_COMBO (group_combo), groups);
    if ((pw_buf = getpwuid (info->statbuf.st_uid)) != NULL)
      {
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (user_combo)->entry),
			    pw_buf->pw_name);
      }
    else
      {
	gchar uid_buf[20];
	g_snprintf (uid_buf, sizeof (uid_buf), "%d",
		    (int) info->statbuf.st_uid);
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (user_combo)->entry),
			    uid_buf);
      }

    if ((grp_buf = getgrgid (info->statbuf.st_gid)) != NULL)
      {
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (group_combo)->entry),
			    grp_buf->gr_name);
      }
    else
      {
	gchar gid_buf[20];
	g_snprintf (gid_buf, sizeof (gid_buf), "%d",
		    (int) info->statbuf.st_gid);
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (group_combo)->entry),
			    gid_buf);
      }

    g_list_free (users);
    g_list_free (groups);
    setpwent ();
    setgrent ();
  }

  recurse_dirs_button = add_check_button (dialog_vbox, "Recurse Directories",
					  FALSE, FALSE, 5, NULL, NULL);

  add_button (action_area, "Ok", TRUE, 0, ok_cb, NULL);
  if (g_list_length (curr_view->iter) > 1)
    add_button (action_area, "Apply To All", TRUE, 0, apply_to_all_cb, NULL);
  add_button (action_area, "Cancel", TRUE, 0, cancel_cb, ownership_dialog);

  gtk_window_set_position (GTK_WINDOW (ownership_dialog), GTK_WIN_POS_CENTER);
  gtk_widget_show (ownership_dialog);
  gtk_grab_add (ownership_dialog);
  gtk_window_set_transient_for (GTK_WINDOW (ownership_dialog),
				GTK_WINDOW (app.main_window));
}
