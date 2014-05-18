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


#include <stdlib.h>
#include <string.h>
#include "vide.h"



static void
execute_com(gchar *command)
{
	GString *expanded_command;

	expanded_command = expand_macros(command);
	if(command == NULL)
	{
		g_string_free(expanded_command, TRUE);
	}
	else
	{
		pipe_and_capture_output_threaded(expanded_command->str);
		g_string_free(expanded_command, TRUE);
	}
}

static void
trash_cb(GtkWidget *widget)
{
  gchar buf[126];
	g_snprintf(buf, 126, "mv %%f %s", cfg.trash);
	execute_com(buf);

}

static void
copy_cb(GtkWidget *widget)
{
  gchar buf[126];

	g_snprintf(buf, 126, "cp %%f %%D");
	execute_com(buf);
}

static void
move_cb(GtkWidget *widget)
{
  gchar buf[126];

	g_snprintf(buf, 126, "mv -f %%f %%D");
	execute_com(buf);

}

static void
rename_cb(GtkWidget *widget)
{
  gchar buf[126];

	g_snprintf(buf, 126, "mv %%f %%{Filename:}");
	execute_com(buf);

}

/* this should be user definable */
static void
make_tar_cb(GtkWidget *widget)
{
	exec_action("tar c %f | gzip > %{Filename for tar.gz}");
}

static void
make_bz2_cb(GtkWidget *widget)
{
	exec_action("tar c %f | bzip2  > %{Filename for tar.bz2}");
}


static void
choose_action_cb(GtkWidget *widget)
{
	  gchar *action, *s;
		gtk_label_get(GTK_LABEL(GTK_MENU_ITEM(widget)->item.bin.child), &s);
		action = g_strdup(s);
		action = g_strdup(s); /* since exec action writes to the string passed */
				  exec_action(action);
		g_free(action);
}


void
dialog_key_cb(GtkWidget *widget, GdkEventKey *event)
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
			case 'l':
				event->keyval = GDK_Right;
				break;
			case 'h':
				event->keyval = GDK_Left;
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
start_netscape(GtkWidget *widget, gpointer data)
{
	gchar com[PATH_MAX];
	gchar *file;

	gtk_clist_get_text(GTK_CLIST(curr_view->clist), curr_view->row, 0, &file);
	g_snprintf(com, sizeof(com), "netscape %s/%s", curr_view->dir, file);
	file_exec(com);
}
void
reuse_netscape(GtkWidget *widget, gpointer data)
{
	gchar com[PATH_MAX];
	gchar *file;

	gtk_clist_get_text(GTK_CLIST(curr_view->clist), curr_view->row, 0, &file);
	g_snprintf(com, sizeof(com), "netscape -remote 'openURL(file:%s/%s)'",
			curr_view->dir, file);
	file_exec(com);

}
void
new_window_netscape(GtkWidget *widget, gpointer data)
{
	gchar com[PATH_MAX];
	gchar *file;

	gtk_clist_get_text(GTK_CLIST(curr_view->clist), curr_view->row, 0, &file);
	g_snprintf(com, sizeof(com),
			"netscape -remote 'openURL(file:%s/%s, new-window)'",
			curr_view->dir, file);
	file_exec(com);

}

void
add_netscape(void)
{
	
	add_menu_item(app.filetype_popup, "Open in Netscape", start_netscape, NULL);
	add_menu_separator(app.filetype_popup);
	add_menu_item(app.filetype_popup, "Reuse Current Netscape Window",
			reuse_netscape, NULL);
	add_menu_separator(app.filetype_popup);
	add_menu_item(app.filetype_popup, "Open in New Netscape Window",
			new_window_netscape, NULL);
	add_menu_separator(app.filetype_popup);
}
void
create_filetype_popup(void)
{

    GtkWidget *menu_item;
		GtkWidget *menu;
		FileInfo *info;

  	app.filetype_popup = gtk_menu_new();

		curr_view->iter = get_selection(curr_view);
		g_return_if_fail(curr_view->iter != NULL);
		curr_view->iter_base = curr_view->iter;
		info = curr_view->iter->data;

		if(S_ISDIR(info->statbuf.st_mode))
		{
			menu_item = add_menu_item(app.filetype_popup, "File Info",
					file_info_cb, NULL);
			add_menu_separator(app.filetype_popup);
			menu_item = add_menu_item(app.filetype_popup, "Permissions",
					permissions_cb, NULL);
			add_menu_separator(app.filetype_popup);
			menu_item = add_menu_item(app.filetype_popup, "User Group",
					ownership_cb, NULL);
			if(!cfg.fileselector)
			{
				add_menu_separator(app.filetype_popup);
				/* add actions here */
				menu_item = add_menu_item(app.filetype_popup, "Make tar.gz",
					make_tar_cb, NULL);
				add_menu_separator(app.filetype_popup);
				menu_item = add_menu_item(app.filetype_popup, "Make tar.bz2",
					make_bz2_cb, NULL);
				add_menu_separator(app.filetype_popup);
			}

		}
		else
		{
			GtkWidget *menu_item;

			gchar *ext, *program, *s, *free_this;

			menu_item = add_menu_item(app.filetype_popup, "File Info",
					file_info_cb, NULL);
			add_menu_separator(app.filetype_popup);

			if(!cfg.fileselector)
			{
    		menu = gtk_menu_new();
		  	gtk_signal_connect(GTK_OBJECT(menu), "key_press_event",
			    	GTK_SIGNAL_FUNC(dialog_key_cb), NULL);
				add_menu_item(menu, "Trash", trash_cb, NULL);
				add_menu_separator(menu);
				add_menu_item(menu, "Copy", copy_cb, NULL);
				add_menu_separator(menu);
				add_menu_item(menu, "Move", move_cb, NULL);
				add_menu_separator(menu);
				add_menu_item(menu, "Rename", rename_cb, NULL);
				menu_item = add_menu_item(app.filetype_popup, "Actions", NULL, NULL);
				gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
				add_menu_separator(app.filetype_popup);
			}

			menu_item = add_menu_item(app.filetype_popup, "Permissions",
					permissions_cb, NULL);
			add_menu_separator(app.filetype_popup);
			if(!cfg.fileselector)
			{
				menu_item = add_menu_item(app.filetype_popup, "Open With",
					create_open_with_dialog, NULL);
				add_menu_separator(app.filetype_popup);
			}

  	  info = gtk_clist_get_row_data(GTK_CLIST(curr_view->clist), curr_view->row);
  	  if((ext = strchr(info->filename, '.')) == NULL)
		  {
				/* if no filetype put vi on menu 
				add_menu_item(app.filetype_popup, cfg.viewer_command,
						choose_action_cb, NULL);
				add_menu_separator(app.filetype_popup);
				*/

        add_menu_item(app.filetype_popup, "Cancel", NULL, NULL);
		    gtk_signal_connect(GTK_OBJECT(app.filetype_popup), "key_press_event",
				  GTK_SIGNAL_FUNC(dialog_key_cb), NULL);
			  return;
		  }


  		do
  		{

    		ext++; /* get rid of the leading '.' */
    		program = free_this = get_programs_for_ext(ext);
    		if (program != NULL)
    		{
      		while ((s = strchr(program, ',')) != NULL)
      		{
        		*s++ = '\0';
						/* special menu for netscape -remote */
						if(strcmp(program, "netscape"))
						{
							add_menu_item(app.filetype_popup, program,
									choose_action_cb, NULL);
							add_menu_separator(app.filetype_popup);
						}
						else
							add_netscape();
        		program = s;
      		}
					if(strcmp(program, "netscape"))
					{
						add_menu_item(app.filetype_popup, program, choose_action_cb, NULL);
        		add_menu_separator(app.filetype_popup);
					}
					else
						add_netscape();

        		add_menu_item(app.filetype_popup, "Cancel", NULL, NULL);
		    		gtk_signal_connect(GTK_OBJECT(app.filetype_popup),
								"key_press_event",
				  	GTK_SIGNAL_FUNC(dialog_key_cb), NULL);

      		free(free_this);
      		return;
     		}
  		} while ((ext = strchr(ext, '.')) != NULL);
		}


    add_menu_item(app.filetype_popup, "Cancel", NULL, NULL);
		gtk_signal_connect(GTK_OBJECT(app.filetype_popup), "key_press_event",
				GTK_SIGNAL_FUNC(dialog_key_cb), NULL);

}
