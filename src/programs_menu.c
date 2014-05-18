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

void
add_program (gchar * name, gchar * action)
{
  Programs program;

  strncpy (program.name, name, sizeof (program.name));
  strncpy (program.action, action, sizeof (program.action));
  cfg.programs = g_list_append (cfg.programs,
				     duplicate (&program,
						sizeof (Programs)));
}

static void
action_cb(GtkWidget *widget, gpointer data)
{
	GString *expanded_com;
	
	expanded_com = expand_macros(data);
	if(expanded_com == NULL)
	{
		g_string_free(expanded_com, TRUE);
	}
	else
	{
		file_exec(expanded_com->str);
		g_string_free(expanded_com, TRUE);
	}
}

void
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
create_programs_menu(void)
{
		GList *tmp;

		app.programs_menu = gtk_menu_new();

		for(tmp = cfg.programs; tmp != NULL; tmp = tmp->next)
		{
			Programs *program = tmp->data;

			add_menu_item(app.programs_menu, program->name,
					action_cb, program->action);
			add_menu_separator(app.programs_menu);
		}
		add_menu_item(app.programs_menu, "Cancel", NULL, NULL);
		gtk_signal_connect(GTK_OBJECT(app.programs_menu), "key_press_event",
				GTK_SIGNAL_FUNC(menu_key_cb), NULL);

}

