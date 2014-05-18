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

#include"vide.h"

void
history_cb(GtkWidget *widget, gpointer data)
{
	change_dir(curr_view, data);

}

void
history_key_cb(GtkWidget *widget, GdkEventKey *event)
{

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
			default:
	  		event->keyval = 0;
	  	break;
			}
    }
  return;
}


void
create_history_menu(FileView *view)
{
	GList *tmp = NULL;

	app.history_menu = gtk_menu_new();
	
	for(tmp = view->menu_history; tmp != NULL; tmp = tmp->next)
	{
		History *history = tmp->data;

		add_menu_item(app.history_menu, history->dir, history_cb, history->dir);
		add_menu_separator(app.history_menu);
	}
	g_list_free(tmp);
	add_menu_item(app.history_menu, "Cancel", NULL, NULL);
	gtk_signal_connect(GTK_OBJECT(app.history_menu), "key_press_event",
			GTK_SIGNAL_FUNC(history_key_cb), NULL);

	gtk_menu_popup(GTK_MENU(app.history_menu), NULL, NULL,
				set_menu_position, NULL, 0, 0);

}

