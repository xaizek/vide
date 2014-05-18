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
delc_cb(GtkWidget *widget, gpointer data)
{
	GList *tmp;

	for(tmp = cfg.command_mode_programs; tmp != NULL; tmp = tmp->next)
	{
		Command_Mode_Programs *program = tmp->data;
		if(!strcmp(program->name, data))
		{
			cfg.command_mode_programs = g_list_remove_link(cfg.command_mode_programs, 
					tmp);
			break;
		}
	}
	write_command_mode_file();
	recreate_main_window();
	g_list_free(tmp);
}

static void
action_cb(GtkWidget *widget, gpointer data)
{
	gchar buffer[1024];
	g_snprintf(buffer, sizeof(buffer), ":%s", (char *)data);
	execute_command(buffer);
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
create_command_mode_menu(void)
{
		GList *tmp;
		gchar label[1024];

		app.command_mode_menu = gtk_menu_new();

		for(tmp = cfg.command_mode_programs; tmp != NULL; tmp = tmp->next)
		{
			Command_Mode_Programs *program = tmp->data;

			g_snprintf(label, sizeof(label), "%s    %s", program->name,
					program->action);
			add_menu_item(app.command_mode_menu, label,
					action_cb, program->name);
			add_menu_separator(app.command_mode_menu);
		}
		add_menu_item(app.command_mode_menu, "Cancel", action_cb, NULL);
		gtk_signal_connect(GTK_OBJECT(app.command_mode_menu), "key_press_event",
				GTK_SIGNAL_FUNC(menu_key_cb), NULL);
		g_list_free(tmp);
}

void
create_delc_menu(void)
{
		GList *tmp;
		gchar label[1024];

		app.delc_menu = gtk_menu_new();

		for(tmp = cfg.command_mode_programs; tmp != NULL; tmp = tmp->next)
		{
			Command_Mode_Programs *program = tmp->data;

			g_snprintf(label, sizeof(label), "%s    %s", program->name,
					program->action);
			add_menu_item(app.delc_menu, label,
					delc_cb, program->name);
			add_menu_separator(app.delc_menu);
		}
		add_menu_item(app.delc_menu, "Cancel", action_cb, NULL);
		gtk_signal_connect(GTK_OBJECT(app.delc_menu), "key_press_event",
				GTK_SIGNAL_FUNC(menu_key_cb), NULL);
		g_list_free(tmp);


}
