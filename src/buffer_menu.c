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
#include "vide.h"


static void
action_cb(GtkWidget *widget, gpointer data)
{
	gtk_notebook_set_page(GTK_NOTEBOOK(app.notebook), GPOINTER_TO_INT(data));
}


static void
menu_key_cb(GtkWidget *widget, GdkEventKey *event)
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
	if(event->keyval == GDK_Return)
	{
		
	}
  return;
}


void
create_buffer_menu(void)
{
		GtkWidget *label;
		ZvtTerm *tmp_term;
		gint page = 1;

		app.buffer_menu = gtk_menu_new();

		
		add_menu_item(app.buffer_menu, "  1 Vide", action_cb, GINT_TO_POINTER(0));
		add_menu_separator(app.buffer_menu);
		while((tmp_term = get_nth_zvt(GTK_NOTEBOOK(app.notebook), page)))
		{
			gchar *text;
			gchar buf[128];

			label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(app.notebook),
						gtk_notebook_get_nth_page(GTK_NOTEBOOK(app.notebook), page));
			gtk_label_get(GTK_LABEL(label), &text);

			if(GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(tmp_term))))
			{
				if(tmp_term == cfg.current_term)
				{
					g_snprintf(buf, sizeof(buf), "* %s", text);
					add_menu_item(app.buffer_menu, buf, action_cb, 
				 			GINT_TO_POINTER(page));
				}
				else
				{
					g_snprintf(buf, sizeof(buf), "  %s", text);
					add_menu_item(app.buffer_menu, buf, action_cb, 
				 			GINT_TO_POINTER(page));
				}

			}
			else
			{
				g_snprintf(buf, sizeof(buf), "  %s", text);
				add_menu_item(app.buffer_menu, buf, action_cb,
							GINT_TO_POINTER(page));
			}

			add_menu_separator(app.buffer_menu);
			page++;
		}

		add_menu_item(app.buffer_menu, "Cancel", NULL, NULL);



		gtk_signal_connect(GTK_OBJECT(app.buffer_menu), "key_press_event",
				GTK_SIGNAL_FUNC(menu_key_cb), NULL);
}


