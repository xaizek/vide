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
#include <unistd.h>
#include "vide.h"


void
permissions_cb (GtkWidget * widget)
{
  FileInfo *info;

  curr_view->iter = get_selection (curr_view);
  g_return_if_fail (curr_view->iter != NULL);

  curr_view->iter_base = curr_view->iter;
  info = curr_view->iter->data;
  create_permissions_dialog (info);
}

void
ownership_cb (GtkWidget * widget)
{
  FileInfo *info;

  curr_view->iter = get_selection (curr_view);
  g_return_if_fail (curr_view->iter != NULL);

  curr_view->iter_base = curr_view->iter;
  info = curr_view->iter->data;
  create_ownership_dialog (info);
}

void
file_info_cb (GtkWidget * widget)
{
  FileInfo *info;

  curr_view->iter = get_selection (curr_view);
  g_return_if_fail (curr_view->iter != NULL);

  curr_view->iter_base = curr_view->iter;
  info = curr_view->iter->data;
  create_file_info_dialog (info);
}

void
rename_cb (GtkWidget * widget)
{
  create_rename_dialog ();
}

void
mkdir_cb (GtkWidget * widget)
{
  create_mkdir_dialog ();
}

void
symlink_as_cb (GtkWidget * widget)
{
  create_symlink_as_dialog ();
}

void
toggle_tag_cb ()
{
  FileInfo *info = gtk_clist_get_row_data (GTK_CLIST (curr_view->clist),
					   curr_view->row);
  gint row = curr_view->row;

  if (g_list_find (curr_view->tagged, info) != NULL)
    {
      curr_view->tagged = g_list_remove (curr_view->tagged, info);
      gtk_clist_set_background (GTK_CLIST (curr_view->clist), row,
				&CLIST_COLOR);
			status_bar_message("File removed from tag list");
    }
  else
    {
      curr_view->tagged = g_list_append (curr_view->tagged, info);
			gtk_clist_unselect_all(GTK_CLIST(curr_view->clist));
      gtk_clist_set_background (GTK_CLIST (curr_view->clist), row,
				&TAG_COLOR);
			status_bar_message("File is tagged");
    }
}


void
command_cb (GtkWidget * widget)
{
  create_command_dialog ();
}

void
configure_cb (GtkWidget * widget)
{
  create_config_dialog (GENERAL_1);
}

void
quit_cb (GtkWidget * widget)
{
  write_filetypes_file ();
  write_bookmarks_file ();
  write_user_commands_file ();
	write_programs_file();
  write_config_file ();
	write_command_mode_file();
	write_term_file();
  gtk_main_quit ();
}


