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
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "vide.h"

void *
duplicate (void *stuff, int size)
{
  void *new_stuff = (void *) malloc (size);
  memcpy (new_stuff, stuff, size);
  return new_stuff;
}

gchar *
str_to_lower (gchar * string)
{
  gchar *new_string;
  int i = 0;

  if (string == NULL)
    return NULL;

  if ((new_string = strdup (string)) == NULL)
    return NULL;

  while ((new_string[i++] = tolower (new_string[i])) != '\0')
    ;

  return new_string;
}

void
set_cursor_watch ()
{
  GdkCursor *cursor = gdk_cursor_new (GDK_WATCH);
  gdk_window_set_cursor (app.main_window->window, cursor);
//  gdk_cursor_destroy (cursor);
}

void
set_cursor_normal ()
{
	/*
  GdkCursor *cursor = gdk_cursor_new (GDK_LEFT_PTR);
  gdk_window_set_cursor (app.main_window->window, cursor);
  gdk_cursor_destroy (cursor);
	*/
/*  while (gtk_events_pending())*/
/*    gtk_main_iteration();*/
}

int
S_ISEXE (mode_t mode)
{
  if ((S_IXUSR & mode) || (S_IXGRP & mode) || (S_IXOTH & mode))
    return 1;
  return 0;
}

int
is_text (gchar * filename)
{
  FILE *pipe;
  gchar line[MAX_LEN];

  g_snprintf (line, sizeof (line), "file \"%s\"", filename);
  if ((pipe = popen (line, "r")) == NULL)
    {
      fprintf (stderr, "Unable to open pipe for file command\n");
      return 0;
    }

  while (fgets (line, MAX_LEN, pipe))
    {
      if (strstr (line, "text"))
	{
	  pclose (pipe);
	  return 1;
	}
    }
  pclose (pipe);
  return 0;
}

void
free_clist_row_data (gpointer data)
{
  if (data != NULL)
    free (data);
}

void
my_glist_free (GList ** list)
{
  GList *tmp;

  for (tmp = *list; tmp != NULL; tmp = tmp->next)
    {
      if (tmp->data != NULL)
	free (tmp->data);
    }

  g_list_free (*list);
  *list = NULL;
}

GList *
string_glist_find (GList * list, gchar * search_text)
{
  GList *tmp;
  gchar *curr;

  for (tmp = list; tmp != NULL; tmp = tmp->next)
    {
      curr = tmp->data;
      if (STREQ (curr, search_text))
	return tmp;
    }
  return NULL;
}

void
untag_all (FileView * view)
{
  GList *tmp = curr_view->tagged;
  for (; tmp != NULL; tmp = tmp->next)
    {
      FileInfo *info = tmp->data;
      gint row = gtk_clist_find_row_from_data (GTK_CLIST (view->clist), info);
      if (row < 0)
	fprintf (stderr, "Trouble in get_selection()\n");
      else
	gtk_clist_set_background (GTK_CLIST (view->clist), row, &CLIST_COLOR);
    }
  g_list_free (curr_view->tagged);
  curr_view->tagged = NULL;
}

GList *
get_selection (FileView * view)
{
  if (curr_view->tagged != NULL)
    return g_list_copy (curr_view->tagged);
  else
    return clist_get_selected_row_data (view->clist);
}

GList *
clist_get_selected_row_data (GtkWidget * clist)
{
  GList *tmp;
  GList *row_data = NULL;

  /* this got rid of a problem when the user uses shift and the arrow keys */
  /* to make a selection */
  gtk_signal_emit_by_name (GTK_OBJECT (clist), "end-selection");

  for (tmp = GTK_CLIST (clist)->selection; tmp != NULL; tmp = tmp->next)
    {
      FileInfo *info =
	gtk_clist_get_row_data (GTK_CLIST (clist), (int) tmp->data);
      row_data = g_list_append (row_data, info);
    }

  return row_data;
}

void
chomp (gchar * text)
{
  if (text[strlen (text) - 1] == '\n')
    text[strlen (text) - 1] = '\0';
}

/* This is the function that replaces %f etc. with their appropriate values
 * The GString returned needs to be freed.
 */
GString *
expand_macros (gchar * text)
{
  GString *command_string = g_string_new ("");
  gchar *s, *free_this, *command_copy;
  FileInfo *info;

  command_copy = g_strdup (text);
  free_this = s = command_copy;
  while ((s = strchr (command_copy, '%')) != NULL)
    {
      *s++ = '\0';
      g_string_append (command_string, command_copy);
      command_copy = s + 1;
      switch (*s)
	{
	case 'f':
	  {
	    GList *tmp;
	    GList *base;

	    tmp = get_selection (curr_view);
	    if (tmp == NULL)
	      {
					status_bar_message("No files selected\n");
					g_free (free_this);
					g_string_free (command_string, TRUE);
					return NULL;
	      }
	    base = tmp;
	    for (; tmp != NULL; tmp = tmp->next)
	      {
		info = tmp->data;
		g_string_sprintfa (command_string, "\"%s\" ", info->filename);
	      }
	    g_list_free (base);
	  }
	  break;
	case 'd':
	  g_string_sprintfa (command_string, "\"%s\"", curr_view->dir);
	  break;
	case 'D':
	  g_string_sprintfa (command_string, "\"%s\"", other_view->dir);
	  break;
	case '{':
	  if ((s = strchr (command_copy, '}')) == NULL)
	    {
	      status_bar_message("No matching '}' found in action text.\n");
	      g_free (free_this);
	      g_string_free (command_string, TRUE);
	      return NULL;
	    }
	  else
	    {
	      gchar *user_input;

	      *s++ = '\0';

				/*%f inside of %{} will put the selected files into the popup entry */
				if((s = strchr(command_copy, '%')) != NULL)
				{
			    GList *tmp;
	    		GList *base;
					gchar *init_text;

	    		tmp = get_selection (curr_view);
	    		if (tmp == NULL)
	      	{
						status_bar_message("No files selected\n");
						g_free (free_this);
						g_string_free (command_string, TRUE);
						return NULL;
	      	}
	    		base = tmp;
	    		for (; tmp != NULL; tmp = tmp->next)
	      	{
						info = tmp->data;
						init_text = g_strdup(info->filename);
	      	}
	    		g_list_free (base);
					*s++ = '\0';
					command_copy = s+1;

					create_user_prompt(command_copy, init_text, &user_input);
				}
				else
	      	create_user_prompt (command_copy, "", &user_input);

	      gtk_main ();
	      command_copy = s;
	      if (user_input != NULL)
				{
		  		g_string_append (command_string, user_input);
		  		g_free (user_input);
					g_free(free_this);
					return command_string;
				}
	      else
				{
		  		g_free (free_this);
		  		g_string_free (command_string, TRUE);
		  		return NULL;
				}
	    }
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

/* Used to show the full command on the status bar without
 * creating the popups
 */
GString *
list_macros (gchar * text)
{
  GString *command_string = g_string_new ("");
  gchar *s, *free_this, *command_copy;
  FileInfo *info;

  command_copy = g_strdup (text);
  free_this = s = command_copy;
  while ((s = strchr (command_copy, '%')) != NULL)
    {
      *s++ = '\0';
      g_string_append (command_string, command_copy);
      command_copy = s + 1;
      switch (*s)
	{
	case 'f':
	  {
	    GList *tmp;
	    GList *base;

	    tmp = get_selection (curr_view);
	    if (tmp == NULL)
	      {
		g_free (free_this);
		g_string_free (command_string, TRUE);
		return NULL;
	      }
	    base = tmp;
	    for (; tmp != NULL; tmp = tmp->next)
	      {
		info = tmp->data;
		g_string_sprintfa (command_string, "\"%s\" ", info->filename);
	      }
	    g_list_free (base);
	  }
	  break;
	case 'd':
	  g_string_sprintfa (command_string, "\"%s\"", curr_view->dir);
	  break;
	case 'D':
	  g_string_sprintfa (command_string, "\"%s\"", other_view->dir);
	  break;
	case '{':
		{
			g_string_append(command_string, "  --User Prompt--");
			g_free(free_this);
			return command_string;
		}
		break;
	default:
	  g_string_append_c (command_string, *s);
	  break;
	}
    }
  g_string_append (command_string, command_copy);
  g_free (free_this);
  return command_string;
}

void
describe_file_size (gchar* buf, int buf_size, FileInfo* file_info)
{
	if (file_info->statbuf.st_size < 10240)	/* less than 10K */
 	{
 		g_snprintf (buf, buf_size, "%d bytes", (int) file_info->statbuf.st_size);
 	}
 	else if (file_info->statbuf.st_size < 1048576)	/* less than a meg */
 	{
 		g_snprintf (buf, buf_size, "%.2f Kbytes",
 			((float) file_info->statbuf.st_size / 1024.0));
 	}
 	else			/* more than a meg */
 	{
 		g_snprintf (buf, buf_size, "%.2f Mbytes",
 			((float) file_info->statbuf.st_size / 1048576.0));
 	}
}
 
void
describe_user_id (gchar* buf, int buf_size, FileInfo* file_info)
{
	struct passwd *pwd_buf;
 
 	if ((pwd_buf = getpwuid (file_info->statbuf.st_uid)) == NULL)
 	{
 		g_snprintf (buf, buf_size, "%d", (int) file_info->statbuf.st_uid);
 	}
 	else
 	{
 		g_snprintf (buf, buf_size, "%s", pwd_buf->pw_name);
 	}
}

void
describe_group_id (gchar* buf, int buf_size, FileInfo* file_info)
{
 	struct group *grp_buf;
 
 	if ((grp_buf = getgrgid (file_info->statbuf.st_gid)) == NULL)
 	{
 		g_snprintf (buf, sizeof (buf), "%d", (int) file_info->statbuf.st_gid);
 	}
 	else
 	{
 		g_snprintf (buf, buf_size, "%s", grp_buf->gr_name);
 	}
}

/* This is based on code by Sergei Gnezdov */
void
show_full_file_properties()
{
	gchar status_str[MAX_LEN];
	FileInfo *info;
	gchar perm_str[11];
	gchar size_str[20];
	struct tm *tm_ptr;
	gchar modified_str[50];
	gchar user_id_str[20];
	gchar group_id_str[20];
						
	info = gtk_clist_get_row_data(GTK_CLIST(curr_view->clist),
			curr_view->row);
	get_perm_string(perm_str, sizeof(perm_str), info->statbuf.st_mode);
	tm_ptr = localtime(&info->statbuf.st_mtime);
	strftime(modified_str, sizeof(modified_str),
				"%a %b %d %H:%M", tm_ptr);
	describe_file_size(size_str, sizeof(size_str), info);
	describe_user_id(user_id_str, sizeof(user_id_str), info);
	describe_group_id(group_id_str, sizeof(group_id_str), info);
	if(!strcmp(info->filename, "../"))
		g_snprintf(status_str, sizeof(status_str),
			"%s  Size: %s  Modified: %s  %s  %s:%s    %d files",
			curr_view->dir, size_str, modified_str, perm_str, 
			user_id_str,group_id_str, GTK_CLIST(curr_view->clist)->rows);
	else
		g_snprintf(status_str, sizeof(status_str),
			"%s  Size: %s  Modified: %s  %s  %s:%s    %d files",
			info->filename, size_str, modified_str, perm_str, 
			user_id_str,group_id_str, GTK_CLIST(curr_view->clist)->rows);

	status_bar_message(status_str);

}
void
show_file_properties()
{
	gchar status_str[MAX_LEN];
	FileInfo *info;
	gchar perm_str[11];
	gchar size_str[20];
	struct tm *tm_ptr;
	gchar modified_str[50];
	gchar user_id_str[20];
	gchar group_id_str[20];
	gint selections = 0;
						
	if((selections = g_list_length(GTK_CLIST(curr_view->clist)->selection)) > 1)
	{
			g_snprintf(status_str, sizeof(status_str), 
				"%d of %d files selected", 
				selections, GTK_CLIST(curr_view->clist)->rows);
	}
	else
	{

		info = gtk_clist_get_row_data(GTK_CLIST(curr_view->clist),
				curr_view->row);
		get_perm_string(perm_str, sizeof(perm_str), info->statbuf.st_mode);
		tm_ptr = localtime(&info->statbuf.st_mtime);
		strftime(modified_str, sizeof(modified_str),
					"%a %b %d %H:%M", tm_ptr);
		describe_file_size(size_str, sizeof(size_str), info);
		describe_user_id(user_id_str, sizeof(user_id_str), info);
		describe_group_id(group_id_str, sizeof(group_id_str), info);
		if(!strcmp(info->filename, "../"))
			g_snprintf(status_str, sizeof(status_str),
				"%s  %s  %s  %s",
				curr_view->dir, size_str, perm_str, 
					user_id_str);
		else
			g_snprintf(status_str, sizeof(status_str),
					"%s  %s  %s  %s",
				info->filename, size_str, perm_str, 
					user_id_str);

	}
	status_bar_message(status_str);

}
