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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "vide.h"

void
create_copy_as_dialog ()
{
  gchar label[NAME_MAX + 32];
  gchar dest[PATH_MAX + NAME_MAX];
  gchar src[PATH_MAX + NAME_MAX];
  gchar *new_name;
  FileInfo *info;
  GList *tmp, *base;
  //guint overwrite_button;
  //gboolean check = cfg.confirm_overwrite;
  //gboolean cancel = FALSE;

  base = tmp = get_selection (curr_view);
  g_return_if_fail (tmp != NULL);

  for (; tmp != NULL; tmp = tmp->next)
    {
      info = tmp->data;
      strncpy (src, info->filename, sizeof (src));
      if (src[strlen (src) - 1] == '/')
	src[strlen (src) - 1] = '\0';

      g_snprintf (label, sizeof (label), "Enter new filename for: %s", src);
      create_user_prompt (label, src, &new_name);
      gtk_main ();

      if (new_name != NULL)
	{
	  g_snprintf (dest, sizeof (dest), "%s/%s", other_view->dir,
		      new_name);
	  g_snprintf (src, sizeof (src), "%s/%s", curr_view->dir,
		      info->filename);
	 	  file_copy (src, dest);
	  g_free (new_name);
	}
    }

  g_list_free (base);
  untag_all (curr_view);
  refresh_list (other_view);
  refresh_list (other_view);
}

void
create_symlink_as_dialog ()
{
  gchar label[NAME_MAX + 32];
  gchar *new_name;
  gchar dest[PATH_MAX + NAME_MAX];
  gchar src[PATH_MAX + NAME_MAX];
  FileInfo *info;
  GList *tmp, *base;

  base = tmp = get_selection (curr_view);
  g_return_if_fail (tmp != NULL);

  for (; tmp != NULL; tmp = tmp->next)
    {
      info = tmp->data;
      strncpy (src, info->filename, sizeof (src));
      if (src[strlen (src) - 1] == '/')
	src[strlen (src) - 1] = '\0';

      g_snprintf (label, sizeof (label), "Enter new filename for: %s", src);
      create_user_prompt (label, src, &new_name);
      gtk_main ();

      if (new_name != NULL)
	{
	  g_snprintf (dest, sizeof (dest), "%s/%s", other_view->dir,
		      new_name);
	  g_snprintf (src, sizeof (src), "%s/%s", curr_view->dir,
		      info->filename);
	  if (file_symlink (src, dest) == -1)
	    status_errno ();
	  g_free (new_name);
	}
    }

  g_list_free (base);
  untag_all (curr_view);
  refresh_list (other_view);
  refresh_list (other_view);
}

void
create_rename_dialog ()
{
  FileInfo *info;
  gchar src[PATH_MAX + NAME_MAX];
  gchar dest[PATH_MAX + NAME_MAX];
  gchar *new_name;
  GList *tmp, *base;
  //guint overwrite_button;
  //gboolean check = cfg.confirm_overwrite;
  //gboolean cancel = FALSE;

  base = tmp = get_selection (curr_view);
  g_return_if_fail (tmp != NULL);

  for (; tmp != NULL; tmp = tmp->next)
    {
      info = tmp->data;
      strncpy (src, info->filename, sizeof (src));
      if (src[strlen (src) - 1] == '/')
	src[strlen (src) - 1] = '\0';

      create_user_prompt ("Enter new file name:", src, &new_name);
      gtk_main ();

      if (new_name != NULL)
	{
	  g_snprintf (dest, sizeof (dest), "%s/%s", curr_view->dir, new_name);
	  g_snprintf (src, sizeof (src), "%s/%s", curr_view->dir,
		      info->filename);
	  file_move (src, dest);
	  g_free (new_name);
	}
    }

  g_list_free (base);
  untag_all (curr_view);
  refresh_list (curr_view);
}

void
create_mkdir_dialog ()
{
  gchar *new_dir;
  gchar path[PATH_MAX];

  create_user_prompt ("Enter name for new directory:", "", &new_dir);
  gtk_main ();

  if (new_dir != NULL)
    {
      g_snprintf (path, sizeof (path), "%s/%s", curr_view->dir, new_dir);
      if (file_mkdir (path) != 0)
	status_errno ();
      g_free (new_dir);
    }

  refresh_list (curr_view);
}

void
create_open_with_dialog ()
{
  gchar *command;

  create_user_prompt ("Enter command:", "", &command);
  gtk_main ();

  if (command != NULL)
    {
      GString *command_line = g_string_new (command);
      GList *base, *tmp;
      base = tmp = get_selection (curr_view);
      for (; tmp != NULL; tmp = tmp->next)
	{
	  FileInfo *info = tmp->data;
	  g_string_sprintfa (command_line, " \"%s\"", info->filename);
	}

      if (strncmp (command_line->str, "x ", 2) == 0)
	create_zterm(command_line->str + 2, TRUE);
      else
	file_exec (command_line->str);
      g_string_free (command_line, TRUE);
      g_free (command);
    }
}

void
create_confirm_del_dialog (gchar * filename, guint * answer)
{
  gchar label_text[MAX_LEN];

  g_snprintf (label_text, sizeof (label_text),
	      "Are you sure you want to delete: %s", filename);
  create_confirm_dialog (label_text, answer);
}

void
create_confirm_overwrite_dialog (gchar * filename, guint * answer)
{
  gchar label_text[MAX_LEN];

  g_snprintf (label_text, sizeof (label_text),
	      "%s already exists! Overwrite this file?", filename);
  create_confirm_dialog (label_text, answer);
}

void
create_command_dialog ()
{
  gchar *command;

  create_user_prompt ("Enter Command:", "", &command);
  gtk_main ();

  if (command != NULL)
    {
      if (strncmp (command, "x ", 2) == 0)
	create_zterm (command + 2, TRUE);
      else
	file_exec (command);
      g_free (command);
    }
}
