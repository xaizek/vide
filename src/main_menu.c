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
#include "vide.h"

static void
help_cb(GtkWidget *widget, gpointer data)
{
	gchar help_command[NAME_MAX];
	g_snprintf(help_command, sizeof(help_command), "%s %s/vide%s.txt",
			cfg.vi_command, cfg.config_dir, VERSION); 
	create_zterm(help_command, TRUE);

}
void
show_hidden_cb (void)
{
	gint last_selected_row;

	last_selected_row = curr_view->row;
  if (cfg.show_hidden)
    cfg.show_hidden = FALSE;
  else
    cfg.show_hidden = TRUE;

	gtk_clist_freeze(GTK_CLIST(curr_view->clist));
	gtk_clist_freeze(GTK_CLIST(other_view->clist));

	load_dir_list(other_view);
	load_dir_list(curr_view);

  focus_on_row (curr_view,
		(last_selected_row < GTK_CLIST (curr_view->clist)->rows
			? last_selected_row
		  : GTK_CLIST (curr_view->clist)->rows - 1));
	gtk_clist_thaw(GTK_CLIST(other_view->clist));
	gtk_clist_thaw(GTK_CLIST(curr_view->clist));
}

static void
goto_bookmark (GtkWidget * menu_item, gchar *value)
{
	GList *tmp;
	gint i;
	gchar *file;

	for(tmp = cfg.bookmarks; tmp != NULL; tmp = tmp->next)
	{
		Bookmarks *bk = tmp->data;
		if(!strcmp(value, bk->mark))
		{
			change_dir(curr_view, bk->dir);
			gtk_clist_freeze(GTK_CLIST(curr_view->clist));
			for(i = 0; i < GTK_CLIST(curr_view->clist)->rows; i++)
			{
				gtk_clist_get_text(GTK_CLIST(curr_view->clist), i, 0, &file);

				if(!strcmp(bk->file, file))
				{
					focus_on_row(curr_view, i);
					gtk_clist_thaw(GTK_CLIST(curr_view->clist));
					return;
				}
			}
			focus_on_row(curr_view, 0);
			gtk_clist_thaw(GTK_CLIST(curr_view->clist));
			return;
		}
	}
	status_bar_message("Mark not found");
	return;
}

/*
 * Keypress Menu Callbacks
 */
void
bookmark_key_cb (GtkWidget * widget, GdkEventKey * event, FileView * view)
{
	if ((event->keyval == 'c') && (event->state & GDK_CONTROL_MASK))
	{
		gtk_signal_emit_by_name(GTK_OBJECT(app.vide_menu_item), "activate-item");
		return;
	}

  if ((event->keyval < 0x100)
      && (event->state == 0 || event->state & GDK_SHIFT_MASK
	  || event->state & GDK_MOD2_MASK))
    {
      switch (event->keyval)
	{

	case 'j':
	  event->keyval = GDK_Down;
	  break;
	case 'k':
	  event->keyval = GDK_Up;
	  break;

	case 'h':
	  event->keyval = GDK_Left;
	  break;
	case 'l':
	  event->keyval = GDK_Right;
	  break;

	default:
	  event->keyval = 0;
	  break;
	}
    }
  return;
}


void
root_cb (void)
{
  /* Should make this user definable for different systems */
  gchar root_command[NAME_MAX];
	g_snprintf(root_command, sizeof(root_command), "%s -e su root -c vide",
			cfg.xterm_command); 
	file_exec(root_command);


}

void
open_trash_cb (void)
{
  change_dir (curr_view, cfg.trash);

}

void
empty_trash_cb (void)
{
	gchar tmp_dir[PATH_MAX];
	g_snprintf(tmp_dir, sizeof(tmp_dir), curr_view->dir);
	
  if (!chdir (cfg.trash))
    {
			/*fix . .. */
      file_exec ("rm -f -r * *. .*");
    }
	chdir(tmp_dir);
}

static void
size_filter_cb(void)
{
	create_size_filter_dialog(curr_view);
}

static void
filename_cb(void)
{
	create_filename_filter_dialog(curr_view);
}

static void
date_filter_cb(void)
{
	create_date_filter_dialog(curr_view);
}

static void
remove_filters_cb(void)
{
	remove_filters(curr_view);
}



static void
edit_bookmarks_cb(GtkWidget *widget)
{
	create_config_dialog(2);
}

gint
sort_bookmarks(gconstpointer a, gconstpointer b)
{
	return strcmp(a, b);
}

void
load_bookmarks ()
{
  GList *tmp;
  GtkWidget *menu_item;


	/* sort marks so that they are listed alphabetically */
	cfg.bookmarks = g_list_sort(cfg.bookmarks, sort_bookmarks);

  for (tmp = cfg.bookmarks; tmp != NULL; tmp = tmp->next)
    {
			gchar bookmark[((2 * PATH_MAX)+5)];
      Bookmarks *bk = tmp->data;
			if(!strcmp(bk->file, "../"))
			{
				g_snprintf(bookmark, sizeof(bookmark), "%s       %s", bk->mark, 
					bk->dir);
			}
			else
			{
				g_snprintf(bookmark, sizeof(bookmark), "%s       %s/%s", bk->mark,
						bk->dir, bk->file);
			}

      menu_item = gtk_menu_item_new_with_label (bookmark);
      gtk_widget_set_name (GTK_WIDGET (menu_item), bookmark);
      gtk_signal_connect (GTK_OBJECT (menu_item), "activate",
			  GTK_SIGNAL_FUNC (goto_bookmark), bk->mark);
      gtk_signal_connect (GTK_OBJECT (menu_item), "key_press_event",
			  GTK_SIGNAL_FUNC (bookmark_key_cb), NULL);
      gtk_menu_append (GTK_MENU (app.bookmark_menu), menu_item);
      gtk_widget_show (menu_item);

    }
	add_menu_separator(app.bookmark_menu);
	add_menu_item(app.bookmark_menu, "Cancel ", NULL, NULL);

}


GtkWidget *
create_main_menu_bar (void)
{

  GtkWidget *main_menu_bar;
  GtkWidget *menu;
	GtkWidget *filter_menu;
	GtkWidget *menu_item;

  main_menu_bar = gtk_menu_bar_new ();

	menu = gtk_menu_new();
	gtk_signal_connect(GTK_OBJECT(menu), "key_press_event",
			GTK_SIGNAL_FUNC(bookmark_key_cb), NULL);

	add_menu_item(menu, "Cancel ", NULL, NULL);
  add_menu_separator (menu);
  add_menu_item (menu, "Become Root", root_cb, NULL);
  add_menu_separator (menu);
  add_menu_item (menu, "Open Trash", open_trash_cb, NULL);
  add_menu_item (menu, "Empty Trash", empty_trash_cb, NULL);
  add_menu_separator (menu);
  add_menu_item (menu, "Quit   Alt Q", quit_cb, NULL);
  app.vide_menu_item = add_submenu (main_menu_bar, "_Vide", menu);

  app.bookmark_menu = gtk_menu_new ();
  gtk_signal_connect (GTK_OBJECT (app.bookmark_menu), "key_press_event",
		      GTK_SIGNAL_FUNC (bookmark_key_cb), NULL);
  add_menu_item (app.bookmark_menu, "Edit Bookmarks", edit_bookmarks_cb,
		 NULL);
  add_menu_separator (app.bookmark_menu);
  app.bookmark_menu_item =
    add_submenu (main_menu_bar, "_Bookmarks", app.bookmark_menu);


  menu = gtk_menu_new ();
  gtk_signal_connect (GTK_OBJECT (menu), "key_press_event",
		      GTK_SIGNAL_FUNC (bookmark_key_cb), NULL);
  add_menu_item(menu, "Show Dot Files", show_hidden_cb, NULL);
  add_menu_separator (menu);

	filter_menu = gtk_menu_new();
	gtk_signal_connect(GTK_OBJECT(filter_menu), "key_press_event",
			GTK_SIGNAL_FUNC(bookmark_key_cb), NULL);
	add_menu_item(filter_menu, "File Name Filter", filename_cb,
			NULL);
	add_menu_separator(filter_menu);
	add_menu_item(filter_menu, "Date Filter", date_filter_cb, NULL);
	add_menu_separator(filter_menu);
	add_menu_item(filter_menu, "Size Filter", size_filter_cb, NULL);
	menu_item = add_menu_item(menu, "Set Filters", NULL, NULL);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), filter_menu);

	add_menu_separator(menu);
	add_menu_item(menu, "Remove Filters", remove_filters_cb, NULL);
	add_menu_separator(menu);
  add_menu_item (menu, "Configure", create_config_dialog, NULL);
	add_menu_separator(menu);
	add_menu_item(menu, "Cancel ", NULL, NULL);
  app.options_menu_item = add_submenu (main_menu_bar, "_Options", menu);

	/*
	menu = gtk_menu_new();
	gtk_signal_connect(GTK_OBJECT(menu), "key_press_event",
			GTK_SIGNAL_FUNC(bookmark_key_cb), NULL);
	add_menu_item(menu, "Open Project", NULL, NULL);
	add_menu_item(menu, "New Project", NULL, NULL);
	add_menu_item(menu, "Import Project", NULL, NULL);
	add_menu_separator(menu);
	add_menu_item(menu, "Close Project", NULL, NULL);
	app.tools_menu_item = add_submenu(main_menu_bar, "_Tools", menu);
	*/

	 
  menu = gtk_menu_new ();
  gtk_signal_connect (GTK_OBJECT (menu), "key_press_event",
		      GTK_SIGNAL_FUNC (bookmark_key_cb), NULL);
	add_menu_item(menu, "Using Vide :h", help_cb, NULL);
	add_menu_separator(menu);
	add_menu_item(menu, "Cancel ", NULL, NULL);
  app.help_menu_item = add_submenu (main_menu_bar, "_Help", menu);
  gtk_menu_item_right_justify (GTK_MENU_ITEM (app.help_menu_item));


  return main_menu_bar;



}
