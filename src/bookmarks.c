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

#include<vide.h>
#include<ctype.h>
#include<string.h>
#include<unistd.h>

void
add_bookmark(gchar *mark, gchar *dir, gchar *file)
{

	g_snprintf(bk.mark, sizeof(bk.mark), mark);
	g_snprintf(bk.dir, sizeof(bk.dir), dir);
	g_snprintf(bk.file, sizeof(bk.file), file);
	cfg.bookmarks = g_list_append(cfg.bookmarks,
			duplicate(&bk, sizeof (Bookmarks)));
}

static void
find_bookmark(FileView *view, gchar *value)
{
	GList *tmp;
	gint i;
	gchar *file;

	for(tmp = cfg.bookmarks; tmp != NULL; tmp = tmp->next)
	{
		Bookmarks *bk = tmp->data;
		if(!strcmp(value, bk->mark))
		{
			change_dir(view, bk->dir);

			gtk_clist_freeze(GTK_CLIST(view->clist));
			for(i = 0; i < GTK_CLIST(view->clist)->rows; i++)
			{
				gtk_clist_get_text(GTK_CLIST(view->clist), i, 0, &file);

				if(!strcmp(bk->file, file))
				{
					focus_on_row(view, i);
					gtk_clist_thaw(GTK_CLIST(view->clist));
					return;
				}
			}
			focus_on_row(view, 0);
			gtk_clist_thaw(GTK_CLIST(view->clist));
			return;
		}
	}
	status_bar_message("Mark not found");
	return;
}

static void
set_bookmark(FileView *view, gchar *value)
{
	gchar *file;
	GList *tmp;


	gtk_clist_get_text(GTK_CLIST(view->clist), view->row, 0, &file);


	/* Check if mark is already used and overwrite it */
	for(tmp = cfg.bookmarks; tmp != NULL; tmp = tmp->next)
	{
		Bookmarks *bk = tmp->data;
		if(!strcmp(value, bk->mark))
		{
			g_snprintf(bk->dir, sizeof(bk->dir), view->dir);
			g_snprintf(bk->file, sizeof(bk->file), file);
			write_bookmarks_file();
			recreate_main_window();
			return;
		}
	}
	g_snprintf(bk.mark, sizeof(bk.mark),  value);
	g_snprintf(bk.dir, sizeof(bk.dir), view->dir);
	g_snprintf(bk.file, sizeof(bk.file), file);
	cfg.bookmarks = g_list_append(cfg.bookmarks,
			duplicate(&bk, sizeof (Bookmarks)));
	write_bookmarks_file();
	recreate_main_window();
}

static void
back_to_normal_mode(FileView *view)
{
	guint new_key_press_signal;

	status_bar_message(" ");

	new_key_press_signal = GPOINTER_TO_INT(gtk_object_get_data(
			GTK_OBJECT(view->clist),"signal_key_id"));
	gtk_signal_disconnect(GTK_OBJECT(view->clist), new_key_press_signal);

	if(cfg.fileselector)
	{
		new_key_press_signal = gtk_signal_connect(GTK_OBJECT(view->clist),
			"key_press_event", GTK_SIGNAL_FUNC(fileselector_key_cb), view);
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
	show_file_properties();

}
static void
set_mark_key_cb(GtkWidget *widget, GdkEventKey *event, FileView *view)
{
	gchar value[10];

	if ((event->keyval == 'c') && (event->state & GDK_CONTROL_MASK))
	{
		event->keyval = 0;
			back_to_normal_mode(view);
			return;
	}
	if(event->keyval == 0)
		return;
	if(event->keyval == GDK_apostrophe)
	{
		event->keyval = 0;
		back_to_normal_mode(view);
		status_bar_message("' is a reserved mark");
		return;
	}


	if ((event->keyval == GDK_Shift_L) || (event->keyval == GDK_Shift_R))
		return;

	if ((event->keyval < 0x100) && (event->state == 0 
				|| event->state & GDK_SHIFT_MASK))
	{
			/* a - z */
		if ((event->keyval > 0x060) && (event->keyval < 0x07b))
		{
			g_snprintf(value, sizeof(value), "%c", event->keyval);
			set_bookmark(view, value);
			event->keyval = 0;
			back_to_normal_mode(view);
			return;
		}
			/* A - Z */
		if ((event->keyval >0x040) && (event->keyval < 0x05a))
		{
			g_snprintf(value, sizeof(value), "%c", event->keyval);
			set_bookmark(view, value);
			event->keyval = 0;
			back_to_normal_mode(view);
			return;
		}
	}
	event->keyval = 0;
	back_to_normal_mode(view);
	status_bar_message("Invalid Mark");
}
static void
find_mark_key_cb(GtkWidget *widget, GdkEventKey *event, FileView *view)
{
	gchar value[10];

	if ((event->keyval == 'c') && (event->state & GDK_CONTROL_MASK))
	{
		event->keyval = 0;
			back_to_normal_mode(view);
			return;
	}
	if ((event->keyval == GDK_Shift_L) || (event->keyval == GDK_Shift_R))
		return;
	if(event->keyval == 0)
		return;

	/* return to last position */
	if(event->keyval == GDK_apostrophe)
	{
		gint tmp = view->last_row;

		change_dir(view, view->last_dir);
		gtk_clist_freeze(GTK_CLIST(view->clist));
		focus_on_row(view, tmp);
		gtk_clist_thaw(GTK_CLIST(view->clist));

		event->keyval = 0;
		back_to_normal_mode(view);
		return;
	}



	if ((event->keyval < 0x100) && (event->state == 0 
				|| event->state & GDK_SHIFT_MASK))
	{
			/* a - z */
		if ((event->keyval > 0x060) && (event->keyval < 0x07b))
		{
			g_snprintf(value, sizeof(value), "%c", event->keyval);
			find_bookmark(view, value);
			event->keyval = 0;
			back_to_normal_mode(view);
			return;
		}
			/* A - Z */
		if ((event->keyval >0x040) && (event->keyval < 0x05a))
		{
			g_snprintf(value, sizeof(value), "%c", event->keyval);
			find_bookmark(view, value);
			event->keyval = 0;
			back_to_normal_mode(view);
			return;
		}
	}
	event->keyval = 0;
	back_to_normal_mode(view);
	status_bar_message("Invalid Mark");
}

void
set_mark(FileView *view)
{
	guint key_press_signal;

	key_press_signal = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(view->clist),  "signal_key_id"));
	gtk_signal_disconnect(GTK_OBJECT(view->clist), key_press_signal);

	key_press_signal = gtk_signal_connect(GTK_OBJECT(view->clist),
			"key_press_event", GTK_SIGNAL_FUNC(set_mark_key_cb), view);
	gtk_object_set_data(GTK_OBJECT(view->clist), "signal_key_id",
			GUINT_TO_POINTER(key_press_signal));

}

void
goto_mark(FileView *view)
{
	guint key_press_signal;

	key_press_signal = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(view->clist),  "signal_key_id"));
	gtk_signal_disconnect(GTK_OBJECT(view->clist), key_press_signal);

	key_press_signal = gtk_signal_connect(GTK_OBJECT(view->clist),
			"key_press_event", GTK_SIGNAL_FUNC(find_mark_key_cb), view);
	gtk_object_set_data(GTK_OBJECT(view->clist), "signal_key_id",
			GUINT_TO_POINTER(key_press_signal));


}
