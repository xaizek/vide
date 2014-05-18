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

#include<sys/param.h>
#if(defined(BSD) && (BSD>=199103)) 
	#include<sys/types.h> /* required for regex.h on FreeBSD 4.2 */
#endif
#include <fnmatch.h>
#include<stdio.h>
#include<string.h>
#include<regex.h>
#include<stdlib.h>
#include<ctype.h>
#include "vide.h"

Pattern pat;

int
search_for_pattern(FileView *view)
{
  gchar *pattern, *s, *free_this = NULL;
  GList *patterns = NULL;
	regex_t re;
  int i = 0;
	int found = 0;

  if((s = g_strdup(view->glob)) == NULL)
		return found;

  free_this = pattern = g_strdup(s);
  while ((s = strchr(pattern, ',')) != NULL)
  {
    *s = '\0';
    patterns = g_list_append(patterns, pattern);
    pattern = s+1;
  }
  patterns = g_list_append(patterns, pattern);

  gtk_clist_unselect_all(GTK_CLIST(view->clist));
	/* search from the current row  */


	if((view->row +1) > (GTK_CLIST(view->clist)->rows -1))
			return found;

  for (i = view->row + 1; i < GTK_CLIST(view->clist)->rows -1; i++)
  {
    GList *tmp = NULL;
		FileInfo *info = gtk_clist_get_row_data(GTK_CLIST(view->clist), i);

    for (tmp = patterns; tmp != NULL; tmp = tmp->next)
    {
      pattern = tmp->data;
			if(regcomp(&re, pattern, REG_EXTENDED) == 0)
			{
      	if (regexec(&re, info->filename, 0, NULL, 0) == 0)
				{
					focus_on_row(view, i);
					found++;
					i = GTK_CLIST(view->clist)->rows;
				}
			}
    }
  }

	regfree(&re);
  g_free(free_this);
	return found;

}

int
search_prev(FileView *view)
{
  gchar *pattern, *s, *free_this;
  GList *patterns = NULL;
	regex_t re;
  int i;
	int found = 0;

  if((s = g_strdup(view->glob)) == NULL)
			return found;

  free_this = pattern = g_strdup(s);
  while ((s = strchr(pattern, ',')) != NULL)
  {
    *s = '\0';
    patterns = g_list_append(patterns, pattern);
    pattern = s+1;
  }
  patterns = g_list_append(patterns, pattern);

  gtk_clist_unselect_all(GTK_CLIST(view->clist));

	if((view->row -1) < 0)
		return found;

  for (i = view->row -1; i > 0; i--)
  {
    FileInfo *info = gtk_clist_get_row_data(GTK_CLIST(view->clist), i);
    GList *tmp;

    for (tmp = patterns; tmp != NULL; tmp = tmp->next)
    {
      pattern = tmp->data;
			if(regcomp(&re, pattern, REG_EXTENDED) == 0)
			{
      	if (regexec(&re, info->filename, 0, NULL, 0) == 0)
				{
					focus_on_row(view, i);
					found++;
					i = 0;
				}
			}
    }

  }

	regfree(&re);
  g_free(free_this);
	return found;

}
void
find_prev(FileView *view)
{
	int found = 0;
	int row = view->row;

	found = search_prev(view);

	if(!found)
	{
		view->row = GTK_CLIST(view->clist)->rows -1;
		found = search_prev(view);
	}
	if(!found)
	{
		view->row = row;
		focus_on_row(view, row);
	}
}


void
find_next(FileView *view)
{
	int found = 0;
	int row = view->row;
	found = search_for_pattern(view);
	if(!found)
	{
		view->row = 0;
		found = search_for_pattern(view);
	}
	if(!found)
	{
		view->row = row;
		gtk_clist_select_row(GTK_CLIST(view->clist), row, 0);
		focus_on_row(view, row);
	}
}

static void
back_to_normal_mode(FileView *view)
{
	guint new_key_press_signal;
	int x;
	gchar status_text[128];

	pat.index = 0;
	for(x = 0; x < PATH_MAX; x++)
		pat.pattern[x] = '\0';


	new_key_press_signal = GPOINTER_TO_INT(gtk_object_get_data(
				GTK_OBJECT(view->clist),"signal_key_id"));
	gtk_signal_disconnect(GTK_OBJECT(view->clist), new_key_press_signal);

	if(cfg.fileselector)
	{
		new_key_press_signal = gtk_signal_connect(GTK_OBJECT(view->clist),
			"key_press_event", GTK_SIGNAL_FUNC(fileselector_key_cb), NULL);
		gtk_object_set_data(GTK_OBJECT(view->clist), "signal_key_id",
			GUINT_TO_POINTER(new_key_press_signal));
	}
	else
	{
		new_key_press_signal = gtk_signal_connect(GTK_OBJECT(view->clist),
			"key_press_event", GTK_SIGNAL_FUNC(file_list_key_press_cb), view);
		gtk_object_set_data(GTK_OBJECT(view->clist), "signal_key_id",
			GUINT_TO_POINTER(new_key_press_signal));
	}

	if(g_list_length(GTK_CLIST(curr_view->clist)->selection) == 0)
	{
			g_snprintf(status_text, sizeof(status_text),
			"No matching files for /%s", view->glob);
	}
	else
	{
			g_snprintf(status_text, sizeof(status_text),
				"%d of %d files selected",
				g_list_length(GTK_CLIST(curr_view->clist)->selection),
				GTK_CLIST(curr_view->clist)->rows);
	}

	status_bar_message(status_text);

}

void
find_pattern(FileView *view)
{
  gchar *pattern, *s, *free_this;
  GList *patterns = NULL;
	regex_t re;
  int i, first;

  if((s = g_strdup(pat.pattern)) == NULL)
			return;

	s++;
	g_snprintf(view->glob, sizeof(view->glob), "%s", s);

  free_this = pattern = g_strdup(s);
  while ((s = strchr(pattern, ',')) != NULL)
  {
    *s = '\0';
    patterns = g_list_append(patterns, pattern);
    pattern = s+1;
  }
  patterns = g_list_append(patterns, pattern);

  gtk_clist_unselect_all(GTK_CLIST(view->clist));
	first = 0;

  for (i = 0; i < GTK_CLIST(view->clist)->rows; i++)
  {
    FileInfo *info = gtk_clist_get_row_data(GTK_CLIST(view->clist), i);
    GList *tmp;

    for (tmp = patterns; tmp != NULL; tmp = tmp->next)
    {
      pattern = tmp->data;
			if(regcomp(&re, pattern, REG_EXTENDED) == 0)
			{
     		if (regexec(&re, info->filename, 0, NULL, 0) == 0)
				{
					if(first)
					{
        		gtk_clist_select_row(GTK_CLIST(view->clist), i, 0);
					}
					else
					{
						first = i;
						focus_on_row(view, i);
					}
				}
			}
    }
  }

	regfree(&re);
	view->row = first;
  g_free(free_this);
	back_to_normal_mode(view);
}





static void
get_pattern(GtkWidget *widget, GdkEventKey *event, FileView *view)
{

	if ((event->keyval == 'c') && (event->state & GDK_CONTROL_MASK))
	{
		event->keyval = 0;
		back_to_normal_mode(view);
		status_bar_message(" ");
		return;
	}
	else if (event->keyval == GDK_Return)
	{
		/* empty pattern use previous pattern for search */
		if(pat.index == 1)
			g_snprintf(pat.pattern, sizeof(pat.pattern), "/%s", view->glob);

		event->keyval = 0;
		find_pattern(view);
		back_to_normal_mode(view);
		return;
	}
	else if(event->keyval == GDK_BackSpace)
	{
		pat.index--;
		pat.pattern[pat.index] = '\0';
		status_bar_message(pat.pattern);
	}
	else if((event->keyval < 0x100) && (event->state == 0
				|| event->state & GDK_SHIFT_MASK))
	{
		pat.pattern[pat.index] = event->keyval;
		pat.index++;
		status_bar_message(pat.pattern);
	}
	else
		event->keyval =0;
}

void
search_mode(FileView *view)
{
	guint key_press_signal;
	pat.index = 0;

	key_press_signal = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(view->clist),  "signal_key_id"));
	gtk_signal_disconnect(GTK_OBJECT(view->clist), key_press_signal);

	key_press_signal = gtk_signal_connect(GTK_OBJECT(view->clist),
			"key_press_event", GTK_SIGNAL_FUNC(get_pattern), view);
	gtk_object_set_data(GTK_OBJECT(view->clist), "signal_key_id",
			GUINT_TO_POINTER(key_press_signal));
	
}
