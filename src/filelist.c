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
#include <sys/time.h>
//#include <sys/types.h>
#include <sys/stat.h>
#include <regex.h>
#include <dirent.h>
//#include <pwd.h>
//#include <grp.h>
#include <unistd.h>
#include "vide.h"
#include <time.h>

typedef struct
{
  gchar path[PATH_MAX];
  gint row;
}
DirHistoryEntry;



static GList *
history_find (GList * list, gchar * path)
{
  GList *tmp;
  DirHistoryEntry *entry;

  for (tmp = list; tmp != NULL; tmp = tmp->next)
    {
      entry = tmp->data;
      if (STREQ (entry->path, path))
	return tmp;
    }
  return NULL;
}

void
get_perm_string (gchar * buf, gint len, mode_t mode)
{
  gchar *perm_sets[] =
    { "---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx" };
  gint u, g, o;

  u = (mode & S_IRWXU) >> 6;
  g = (mode & S_IRWXG) >> 3;
  o = (mode & S_IRWXO);

  g_snprintf (buf, len, "-%s%s%s", perm_sets[u], perm_sets[g], perm_sets[o]);

  if (S_ISLNK (mode))
    buf[0] = 'l';
  else if (S_ISDIR (mode))
    buf[0] = 'd';
  else if (S_ISBLK (mode))
    buf[0] = 'b';
  else if (S_ISCHR (mode))
    buf[0] = 'c';
  else if (S_ISFIFO (mode))
    buf[0] = 'f';
  else if (S_ISSOCK (mode))
    buf[0] = 's';

  if (mode & S_ISVTX)
    buf[9] = (buf[9] == '-') ? 'T' : 't';
  if (mode & S_ISGID)
    buf[6] = (buf[6] == '-') ? 'S' : 's';
  if (mode & S_ISUID)
    buf[3] = (buf[3] == '-') ? 'S' : 's';
}

gint is_dir (FileInfo * info)
{
  if (S_ISDIR (info->statbuf.st_mode))
    return 1;
  if (S_ISLNK (info->statbuf.st_mode))
    {
      struct stat statbuf;
      stat (info->filename, &statbuf);
      if (S_ISDIR (statbuf.st_mode))
	return 1;
    }

  return 0;
}

void
change_dir (FileView * view, gchar * path)
{
	GList *tmp;
	History history;
	gint list_length = 0;
	static gint i = 0;

	/* change the cursor to stop watch for a large directory */


  if (access (path, F_OK) != 0)
    {
      status_bar_message ("That directory does not exist");
      return;
    }
  if (access (path, R_OK) != 0)
    {
      status_bar_message ("You do not have read access on that directory");
      return;
    }
  if (chdir (path))
    {
      status_errno ();
			return;
    }

	g_snprintf(history.dir, sizeof(history.dir), view->dir);
	view->menu_history = g_list_prepend(view->menu_history, 
			duplicate(&history, sizeof(History)));
	if((list_length = g_list_length(view->menu_history)) > 10)
		view->menu_history = g_list_remove_link(view->menu_history,
				g_list_last(view->menu_history));
	/* Keeps the startup dir from being listed twice */ 
	if(i < 2)
	{
		view->menu_history = g_list_remove_link(view->menu_history,
				g_list_last(view->menu_history));
		i++;
	}


	strncpy(view->last_dir, view->dir, PATH_MAX);
	view->last_row = view->row;
      /* Save the last row before we reload the list */
      tmp = history_find (view->dir_history, view->dir);
      if (tmp != NULL)
			{
	  		DirHistoryEntry *entry = tmp->data;
	  		entry->row = view->row;
			}
      getcwd (view->dir, PATH_MAX);
      load_dir_list (view);

	tmp = history_find(view->dir_history, view->dir);
  if (tmp != NULL)
    {
      DirHistoryEntry *entry = tmp->data;

			if(entry->row <= (GTK_CLIST(view->clist)->rows))
			{
				focus_on_row(view, entry->row);
			}
			else
			{
				focus_on_row(view, 0);
			}
      view->dir_history = g_list_remove_link (view->dir_history, tmp);
      view->dir_history = g_list_prepend (view->dir_history, entry);
      g_list_free(tmp);
    }
  else
    {
      DirHistoryEntry entry;

			gtk_clist_select_row(GTK_CLIST(view->clist), 0, 0);
			gtk_widget_grab_focus(view->clist);
      strncpy (entry.path, view->dir, sizeof (entry.path));
      entry.row = 0;

      view->dir_history =
			g_list_prepend (view->dir_history,
			duplicate (&entry, sizeof (DirHistoryEntry)));
      if (g_list_length (view->dir_history) > cfg.dir_history_max_length)
			{
	  		tmp = g_list_nth (view->dir_history, cfg.dir_history_max_length);
	  		view->dir_history = g_list_remove_link (view->dir_history, tmp);
	  		g_list_free (tmp);
			}
			focus_on_row(view, 0);
    }

}

gint name_sort (GtkCList * clist, gconstpointer row1, gconstpointer row2)
{
  FileInfo *info1, *info2;

  info1 = ((GtkCListRow *) row1)->data;
  info2 = ((GtkCListRow *) row2)->data;

  if (STREQ (info1->filename, "../"))
    return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
  if (STREQ (info2->filename, "../"))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);

  if (is_dir (info1))
    {
      if (is_dir (info2))
	return strcmp (info1->filename, info2->filename);
      else
	return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
    }

  if (is_dir (info2))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);

  return strcmp (info1->filename, info2->filename);
}

gint size_sort (GtkCList * clist, gconstpointer row1, gconstpointer row2)
{
  FileInfo *info1, *info2;

  info1 = ((GtkCListRow *) row1)->data;
  info2 = ((GtkCListRow *) row2)->data;

  if (STREQ (info1->filename, "../"))
    return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
  if (STREQ (info2->filename, "../"))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);

  if (is_dir (info1))
    {
      if (is_dir (info2))
	return (info1->statbuf.st_size - info2->statbuf.st_size);
      else
	return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
    }

  if (is_dir (info2))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);

  return (info1->statbuf.st_size - info2->statbuf.st_size);
}

gint date_sort (GtkCList * clist, gconstpointer row1, gconstpointer row2)
{
  FileInfo *info1, *info2;

  info1 = ((GtkCListRow *) row1)->data;
  info2 = ((GtkCListRow *) row2)->data;

  if (STREQ (info1->filename, "../"))
    return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
  if (STREQ (info2->filename, "../"))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);

  if (is_dir (info1))
    {
      if (is_dir (info2))
	return (info1->statbuf.st_mtime - info2->statbuf.st_mtime);
      else
	return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
    }

  if (is_dir (info2))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);

  return (info1->statbuf.st_mtime - info2->statbuf.st_mtime);
}

gint perm_sort (GtkCList * clist, gconstpointer row1, gconstpointer row2)
{
  FileInfo *info1, *info2;

  info1 = ((GtkCListRow *) row1)->data;
  info2 = ((GtkCListRow *) row2)->data;

  if (STREQ (info1->filename, "../"))
    return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
  if (STREQ (info2->filename, "../"))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);

  if (is_dir (info1))
    {
      if (is_dir (info2))
	return (info1->statbuf.st_mode - info2->statbuf.st_mode);
      else
	return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
    }

  if (is_dir (info2))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);

  return (info1->statbuf.st_mode - info2->statbuf.st_mode);
}

gint user_sort (GtkCList * clist, gconstpointer row1, gconstpointer row2)
{
  FileInfo *info1, *info2;

  info1 = ((GtkCListRow *) row1)->data;
  info2 = ((GtkCListRow *) row2)->data;

  if (STREQ (info1->filename, "../"))
    return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
  if (STREQ (info2->filename, "../"))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);

  if (is_dir (info1))
    {
      if (is_dir (info2))
	return (info1->statbuf.st_uid - info2->statbuf.st_uid);
      else
	return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
    }

  if (is_dir (info2))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);

  return (info1->statbuf.st_uid - info2->statbuf.st_uid);
}

gint group_sort (GtkCList * clist, gconstpointer row1, gconstpointer row2)
{
  FileInfo *info1, *info2;

  info1 = ((GtkCListRow *) row1)->data;
  info2 = ((GtkCListRow *) row2)->data;

  if (STREQ (info1->filename, "../"))
    return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
  if (STREQ (info2->filename, "../"))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);

  if (is_dir (info1))
    {
      if (is_dir (info2))
	return (info1->statbuf.st_gid - info2->statbuf.st_gid);
      else
	return (clist->sort_type == GTK_SORT_ASCENDING ? -1 : 1);
    }

  if (is_dir (info2))
    return (clist->sort_type == GTK_SORT_ASCENDING ? 1 : -1);

  return (info1->statbuf.st_gid - info2->statbuf.st_gid);
}

void
sort_list (FileView * view,
	   GtkCListCompareFunc compare_func, GtkSortType direction, gint col)
{
  gtk_widget_hide (view->sort_arrows[GTK_CLIST (view->clist)->sort_column]);
  gtk_widget_show (view->sort_arrows[col]);
  gtk_arrow_set (GTK_ARROW (view->sort_arrows[col]),
		 (direction == GTK_SORT_ASCENDING
		  ? GTK_ARROW_DOWN : GTK_ARROW_UP), GTK_SHADOW_IN);

  gtk_clist_set_compare_func (GTK_CLIST (view->clist), compare_func);
  gtk_clist_set_sort_type (GTK_CLIST (view->clist), direction);
  gtk_clist_set_sort_column (GTK_CLIST (view->clist), col);
  gtk_clist_sort (GTK_CLIST (view->clist));
}

void
focus_on_row (FileView * view, gint row)
{
  gtk_clist_freeze (GTK_CLIST (view->clist));
  gtk_clist_unselect_all (GTK_CLIST (view->clist));
	if(gtk_clist_get_selectable(GTK_CLIST(view->clist), row))
	{
  	gtk_clist_select_row (GTK_CLIST (view->clist), row, 0);
	  GTK_CLIST (view->clist)->focus_row = row;
  	gtk_widget_draw_focus (view->clist);
  	gtk_clist_moveto (GTK_CLIST (view->clist), row, 0, 0.5, 0.0);
	}
	else
	{
		gtk_clist_select_row(GTK_CLIST(view->clist), 0, 0);
	  GTK_CLIST (view->clist)->focus_row = 0;
  	gtk_widget_draw_focus (view->clist);
  	gtk_clist_moveto (GTK_CLIST (view->clist), 0, 0, 0.5, 0.0);
	}
  gtk_clist_thaw (GTK_CLIST (view->clist));
	show_file_properties();
}

void
goto_row (GtkWidget * clist, gint row)
{
  gtk_clist_freeze (GTK_CLIST (clist));
  gtk_clist_unselect_all (GTK_CLIST (clist));
  gtk_clist_select_row (GTK_CLIST (clist), row, 0);
  GTK_CLIST (clist)->focus_row = row;
  gtk_clist_moveto (GTK_CLIST (clist), row, 0, 0.5, 0.0);
  gtk_clist_thaw (GTK_CLIST (clist));
}

void
set_filter_menu_active (FileView * view)
{
	/*
  GtkStyle *style = gtk_style_new ();
  GdkColor red;

  gdk_color_parse ("red", &red);
  style->fg[GTK_STATE_NORMAL] = red;
  style->fg[GTK_STATE_PRELIGHT] = red;

  gtk_widget_set_style (GTK_MENU_ITEM (view->filter_menu_item)->item.bin.
			child, style);
			*/
}

void
remove_filters (FileView * view)
{
  view->filename_filter.active = FALSE;
  view->size_filter.active = FALSE;
  view->date_filter.active = FALSE;

  refresh_list (view);
}

void
initialize_filters (FileView * view)
{
  view->filter_directories = FALSE;

  view->filename_filter.active = TRUE;
	view->filename_filter.invert_mask = cfg.invert_filter;
  strncpy (view->filename_filter.pattern, cfg.start_filter,
	   sizeof (view->filename_filter.pattern));

  view->size_filter.active = FALSE;
  view->size_filter.size = 0;
  view->size_filter.op = GT;

  view->date_filter.active = FALSE;
  view->date_filter.time = time (NULL);
  view->date_filter.time_type = ATIME;
  view->date_filter.op = GT;
}


static gboolean
match_filename_filter (FileView * view, FileInfo * info)
{
	gchar  *pattern, *free_this;
	regex_t re;

	if(is_dir(info) && !view->filter_directories)
		return TRUE;

	free_this = pattern = strdup(view->filename_filter.pattern);

	if(regcomp(&re, pattern, REG_EXTENDED) == 0)
	{
		if(regexec(&re, info->filename, 0, NULL, 0) == 0)
		{
			free(free_this);
			regfree(&re);
			return !view->filename_filter.invert_mask;
		}

		free(free_this);
		regfree(&re);
		return view->filename_filter.invert_mask;
	}
	regfree(&re);

	return TRUE;

	/* glob matching instead of regular expression
  gchar *s, *pattern, *free_this;
  gboolean result;

  if (is_dir (info) && !view->filter_directories)
    return TRUE;

  free_this = pattern = strdup (view->filename_filter.pattern);
  while ((s = strchr (pattern, ',')) != NULL)
    {
      *s = '\0';
      if (gtk_pattern_match_simple (pattern, info->filename))
	{
	  free (free_this);
	  return !view->filename_filter.invert_mask;
	}
      pattern = s + 1;
    }

  result = gtk_pattern_match_simple (pattern, info->filename);
  free (free_this);
  return (view->filename_filter.invert_mask ? !result : result);
	*/
}

static gboolean
match_size_filter (FileView * view, FileInfo * info)
{
  if (is_dir (info) && !view->filter_directories)
    return TRUE;

  switch (view->size_filter.op)
    {
    case GT:
      return (info->statbuf.st_size > view->size_filter.size);
      break;
    case LT:
      return (info->statbuf.st_size < view->size_filter.size);
      break;
    case EQ:
      return (info->statbuf.st_size == view->size_filter.size);
      break;
    default:
      return TRUE;
    }

  /* this can never happen */
  return TRUE;
}

static gboolean
match_date_filter (FileView * view, FileInfo * info)
{
  if (is_dir (info) && !view->filter_directories)
    return TRUE;

  switch (view->date_filter.time_type)
    {
    case ATIME:
      if (view->date_filter.op == GT)
	return (difftime (view->date_filter.time, info->statbuf.st_atime) <
		0);
      else
	return (difftime (view->date_filter.time, info->statbuf.st_atime) >
		0);
      break;
    case MTIME:
      if (view->date_filter.op == GT)
	return (difftime (view->date_filter.time, info->statbuf.st_mtime) <
		0);
      else
	return (difftime (view->date_filter.time, info->statbuf.st_mtime) >
		0);
      break;
    case CTIME:
      if (view->date_filter.op == GT)
	return (difftime (view->date_filter.time, info->statbuf.st_ctime) <
		0);
      else
	return (difftime (view->date_filter.time, info->statbuf.st_ctime) >
		0);
      break;
    default:
      return TRUE;
    }

  /* this cant happen */
  return TRUE;
}

/* NOTE: This function will cause curr_view and other_view to switch if */
/* curr_view isn't the view passed in */
void
refresh_list (FileView * view)
{
  GtkAdjustment *vadj;
  gfloat scrollbar_pos;
  gint last_selected_row;

  g_return_if_fail (view != NULL);

  vadj = gtk_clist_get_vadjustment (GTK_CLIST (view->clist));
  scrollbar_pos = vadj->value;
  last_selected_row = view->row;

  gtk_clist_freeze (GTK_CLIST (view->clist));
  load_dir_list (view);
/*  gtk_adjustment_set_value (GTK_ADJUSTMENT (vadj), scrollbar_pos);
  focus_on_row (view,
		(last_selected_row < GTK_CLIST (view->clist)->rows
		 ? last_selected_row : GTK_CLIST (view->clist)->rows - 1));*/
  gtk_clist_thaw (GTK_CLIST (view->clist));
}

/* NOTE: This function will cause curr_view and other_view to switch if */
/* curr_view isn't the view passed in */
void
load_dir_list (FileView * view)
{
  DIR *dp;
  struct dirent *entry;
  struct stat stat_buf;
  FileInfo info;


  g_return_if_fail (view != NULL);


  stat (view->dir, &stat_buf);


  if ((dp = opendir (view->dir)) == NULL)
    {
			g_snprintf(view->dir, sizeof(view->dir), "%s", getenv("HOME"));
      fprintf (stderr, "Error opening directory going home now.\n");
			create_error_dialog("Error opening directory going home now.\n");
      return;
    }
  view->dir_mtime = stat_buf.st_mtime;

  gtk_clist_freeze (GTK_CLIST (view->clist));
  gtk_clist_unselect_all (GTK_CLIST (view->clist));
  gtk_clist_clear (GTK_CLIST (view->clist));

  chdir (view->dir);
  while ((entry = readdir (dp)) != NULL)
    {
      FileInfo *this_info;
      struct tm *tm_ptr;
      gchar size_buf[20];
      gchar modified_buf[25];
      gchar accessed_buf[25];
      gchar changed_buf[25];
      gchar perm_buf[11];
      gchar uid_buf[20];
      gchar gid_buf[20];
      gchar *buf[MAX_COLUMNS];
      int row;

      if (STREQ (entry->d_name, "."))
	continue;
      if (((entry->d_name[0] == '.') && (!STREQ (entry->d_name, "..")))
	  && !cfg.show_hidden)
	continue;
      lstat (entry->d_name, &(info.statbuf));
      strncpy (info.filename, entry->d_name, sizeof (info.filename));
      if (is_dir (&info))
	strcat (info.filename, "/");

      if (view->filename_filter.active
	  && !match_filename_filter (view, &info))
	continue;
      if (view->size_filter.active && !match_size_filter (view, &info))
	continue;
      if (view->date_filter.active && !match_date_filter (view, &info))
	continue;

      this_info = duplicate (&info, sizeof (FileInfo));

      buf[0] = this_info->filename;

			describe_file_size(size_buf, sizeof(size_buf), this_info);
      buf[1] = size_buf;

      tm_ptr = localtime (&(this_info->statbuf.st_mtime));
      strftime (modified_buf, sizeof (modified_buf), "%b  %d %H:%M", tm_ptr);
      buf[2] = modified_buf;

      tm_ptr = localtime (&(this_info->statbuf.st_atime));
      strftime (accessed_buf, sizeof (accessed_buf), "%b  %d %H:%M", tm_ptr);
      buf[3] = accessed_buf;

      tm_ptr = localtime (&(this_info->statbuf.st_ctime));
      strftime (changed_buf, sizeof (changed_buf), "%b  %d %H:%M", tm_ptr);
      buf[4] = changed_buf;

      get_perm_string (perm_buf, sizeof (perm_buf),
		       this_info->statbuf.st_mode);
      buf[5] = perm_buf;

			describe_user_id(uid_buf, sizeof(uid_buf), this_info);
			buf[6] = uid_buf;

			describe_group_id(gid_buf, sizeof(gid_buf), this_info);
			buf[7] = gid_buf;


      row = gtk_clist_append (GTK_CLIST (view->clist), buf);
      gtk_clist_set_row_data_full (GTK_CLIST (view->clist), row,
				   this_info, free_clist_row_data);
      switch (this_info->statbuf.st_mode & S_IFMT)
			{
				case S_IFLNK:
	  			gtk_clist_set_foreground (GTK_CLIST (view->clist), row, &LNK_COLOR);
	  			break;
				case S_IFDIR:
	  			gtk_clist_set_foreground (GTK_CLIST (view->clist), row, &DIR_COLOR);
	  			break;
				case S_IFCHR:
				case S_IFBLK:
	  			gtk_clist_set_foreground (GTK_CLIST (view->clist), row, &DEV_COLOR);
	  			break;
				case S_IFSOCK:
	  			gtk_clist_set_foreground (GTK_CLIST (view->clist), row,
				    	&SOCK_COLOR);
	  			break;
				case S_IFREG:
	  			if (S_ISEXE (this_info->statbuf.st_mode))
	    			gtk_clist_set_foreground (GTK_CLIST (view->clist), row,
				      	&EXE_COLOR);
	  			break;
				default:
	  			break;
			}
    }
  closedir (dp);

  gtk_clist_sort (GTK_CLIST (view->clist));
	gtk_label_set_text(GTK_LABEL(view->dir_label), view->dir);
  gtk_clist_thaw (GTK_CLIST (view->clist));

  g_list_free (view->tagged);
  view->tagged = NULL;
}


static void
view_file (gchar * filename)
{
  gchar command[2 * NAME_MAX];

  g_return_if_fail (filename != NULL);
	/*
	if(cfg.use_vi_server)
	{
		start_vi_server(filename);
		return;
	}
	if(cfg.use_vimclient)
	{
		call_vimclient(filename);
		return;
	}
	*/

	g_snprintf(command, sizeof(command), "%s \"%s\"", cfg.vi_command, filename);
	create_zterm(command, TRUE);
}

void
exec_action (gchar * action)
{
  FileInfo *info;
  GString *command;

  g_return_if_fail (action != NULL);

  if (strchr (action, '%') == NULL)
    {
      GList *tmp, *base;
      tmp = get_selection (curr_view);
      g_return_if_fail (tmp != NULL);
      base = tmp;

      command = g_string_sized_new (MAX_LEN);
      g_string_assign (command, action);

      for (; tmp != NULL; tmp = tmp->next)
			{
	  		info = tmp->data;
	  		g_string_sprintfa (command, " \"%s\"", info->filename);
			}
      if (strncmp (command->str, "x ", 2) == 0)
			{
				create_zterm(command->str + 2, TRUE);
			}
			else if(strncmp(command->str, "vi ", 3) == 0)
			{
				gchar buf[1024];
				g_snprintf(buf, sizeof(buf), "%s \"%s\"", cfg.vi_command, info->filename);
				create_zterm(buf, TRUE);
			}
      else
				file_exec (command->str);
      g_list_free (base);
      g_string_free (command, TRUE);
    }
  else
    {
      if ((command = expand_macros (action)) != NULL)
			{
	  		if (strncmp (command->str, "x ", 2) == 0)
					create_zterm(command->str + 2, TRUE);
	  		else
	    		file_exec (command->str);
	  		g_string_free (command, TRUE);
			}
    }
}

void
handle_file (FileView * view)
{
  FileInfo *info;
  gchar *ext, *program;
	/*
	gchar buf[PATH_MAX];
	*/


  g_return_if_fail (view != NULL);
  if (view->tagged != NULL)
    info = view->tagged->data;
  else
    info = gtk_clist_get_row_data (GTK_CLIST (view->clist), view->row);
  g_return_if_fail (info != NULL);


  if (is_dir (info))
    {
      gchar path[PATH_MAX];


      if (strcmp (info->filename, "../") == 0)
			{
				change_dir(view, "..");
	  		return;
			}
      g_snprintf (path, sizeof (path), "%s/%s", view->dir, info->filename);
      change_dir (view, path);
      return;
    }

  ext = strchr (info->filename, '.');
  if (ext == NULL)		/* no extension */
    {
      if (S_ISEXE (info->statbuf.st_mode))
			{
	  		gchar command_line[NAME_MAX + 2];
	  		g_snprintf (command_line, sizeof (command_line), "./%s",
		      info->filename);
	  		file_exec (command_line);
	  		return;
			}
			else
			{
			view_file(info->filename);
      return;
			}
    }

  if (ext == info->filename)	/* its a dot file */
    {
	  	view_file (info->filename);
      return;
    }

  /* the file has an extension */
  do
    {
      ext++;			/* get rid of the . prefix */
      program = get_default_program_for_ext (ext);
			if(program == NULL)
			{
				view_file(info->filename);
				free(program);
				return;
			}
      if (program != NULL)
			{
	  		exec_action (program);
	  		free (program);
	  		return;
			}
    }
  while ((ext = strchr (ext, '.')) != NULL);

  /* Didn't find any matching extensions */
  ext = strrchr (info->filename, '.') + 1;
  ext = str_to_lower (ext);

  /* use vi as default on all files */
  /* that don't have a specific file type */
  view_file (info->filename);

  free (ext);
}
