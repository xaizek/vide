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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "vide.h"


Command com;

void
add_command(gchar *name, gchar *action)
{
	Command_Mode_Programs cmp;

	strncpy(cmp.name, name, sizeof(cmp.name));
	strncpy(cmp.action, action, sizeof(cmp.action));
	cfg.command_mode_programs = g_list_append(cfg.command_mode_programs,
			duplicate (&cmp, sizeof (Command_Mode_Programs)));
}

static void
reset_signals()
{
	guint new_key_press_signal;
	int x;

	com.index = 0;
	for(x = 0; x < 1024; x++)
		com.buffer[x] = '\0';

	new_key_press_signal = GPOINTER_TO_INT(gtk_object_get_data(
				GTK_OBJECT(curr_view->clist),"signal_key_id"));
	gtk_signal_disconnect(GTK_OBJECT(curr_view->clist), new_key_press_signal);

	new_key_press_signal = gtk_signal_connect(GTK_OBJECT(curr_view->clist),
			"key_press_event", GTK_SIGNAL_FUNC(file_list_key_press_cb), curr_view);
	gtk_object_set_data(GTK_OBJECT(curr_view->clist), "signal_key_id",
			GUINT_TO_POINTER(new_key_press_signal));

}


static void
back_to_normal_mode(FileView *view)
{
	int x;
	//gchar status_text[128];

	com.index = 0;
	for(x = 0; x < 1024; x++)
		com.buffer[x] = '\0';

	/*
	g_snprintf(status_text, sizeof(status_text),
			"%d of %d files selected",
			g_list_length(GTK_CLIST(curr_view->clist)->selection),
			GTK_CLIST(curr_view->clist)->rows);
	gtk_label_set_text(GTK_LABEL(app.status_bar), status_text);
	*/
	show_file_properties();
	reset_signals();

}


void
execute_command(gchar *action)
{
	gchar *command = strdup(action);
	gchar *arg1 = NULL;
	gchar *arg2 = NULL;
	gchar *arg3 = NULL;
	gint arguments = 0;
	gchar buf[PATH_MAX];
	gchar *file = NULL;
	GList *tmp1;
	GString *ex_command;



	g_snprintf(com.buffer, sizeof(com.buffer), "%s", command);
	command++;
	
	if(!strstr(command, " ")) /* only one argument */
	{
		arg1 = g_strdup(command);
		arguments = 1;
	}
	else /* at least two arguments */
	{
		arg2 = (strchr(command, ' ') +1);
		arg1 = g_strndup(command,(strlen(command) - strlen(arg2) -1));
		arguments = 2;

		if(strstr(arg2, " ")) /* three arguments */
		{
			arg3 =(strchr(arg2, ' ') +1);
			arg2 = g_strndup(arg2, (strlen(arg2) - strlen(arg3) -1));
			arguments = 3;
		}
	}

	/* First selected file */
	gtk_clist_get_text(GTK_CLIST(curr_view->clist), curr_view->row, 0, &file);
	/* All of the selected files */
	ex_command = expand_macros("%f");

/*
 * Expand the macros for user defined commands
 */

	for(tmp1 = cfg.command_mode_programs; tmp1 != NULL; tmp1 = tmp1->next)
	{

		Command_Mode_Programs *program = tmp1->data;

		if(!strcmp(program->name, command))
		{
			GString *tmp_command;

			tmp_command = expand_macros(program->action);

			if(tmp_command != NULL)
			{
				g_snprintf(com.buffer, sizeof(com.buffer), ":%s", tmp_command->str);
				g_string_free(tmp_command, TRUE);
			}
		}
	}
	g_list_free(tmp1);
	

	/* :com */
	if(!strcmp(arg1,"com"))
	{

		if(arguments == 3)
		{
			GList *tmp = NULL;
			Command_Mode_Programs cmp;
			gchar *reserved[] = {"com", "delc", "exe", "/", "cd", "0", "!", "sh",
				"mark","his", "x", "e", "sb", "sp", "on", "only", "set filter", "set",
			"nofil", "nofilter", "in", "invert", "fil", "filter", "vs", "ls", "pw","pwd"};
			int index;

			/* check for commands that should not be redefined  */

			for(index = 0; index < 28; index++)
			{
				if(!strcmp(arg2, reserved[index]))
				{
					gchar buf[48];
					back_to_normal_mode(curr_view);
					g_snprintf(buf, sizeof(buf), "%s is a reserved command name.", 
							reserved[index]);
					create_error_dialog(buf);
					return;
				}
			}
			/* check for duplicate command names and remove */
			for(tmp = cfg.command_mode_programs; tmp != NULL; tmp = tmp->next)
			{
				Command_Mode_Programs *program = tmp->data;

				if(!strcmp(program->name, arg2))
				{
					cfg.command_mode_programs = g_list_remove_link(
							cfg.command_mode_programs,
							tmp);
					break;
				}
			}
			g_list_free(tmp);
			if(arguments == 3)
			{
				strncpy(cmp.name, arg2, sizeof(cmp.name));

				strncpy(cmp.action, arg3, sizeof(cmp.action));
				cfg.command_mode_programs = g_list_append(cfg.command_mode_programs,
					duplicate (&cmp, sizeof (Command_Mode_Programs)));


				write_command_mode_file();
				recreate_main_window();
			}
		}
		else
		{
			create_command_mode_menu();
			gtk_menu_popup(GTK_MENU(app.command_mode_menu), NULL, NULL,
					set_menu_position, NULL, 0, 0);
			back_to_normal_mode(curr_view);
			return;
		}
	}

	/* need to reparse commands after expansion */
	command = strdup(com.buffer);
	command++;

	if(!strstr(command, " ")) 
		arg1 = g_strdup(command);
	else 
	{
		arg2 = (strchr(command, ' ') +1);
		arg1 = g_strndup(command,(strlen(command) - strlen(arg2) -1));
		arguments = 2;

		if(strstr(arg2, " ")) 
		{
			arg3 =(strchr(arg2, ' ') +1);
			arg2 = g_strndup(arg2, (strlen(arg2) - strlen(arg3) -1));
			arguments = 3;
		}
	}

	/* exe */
	if(!strcmp(arg1, "exe"))
	{
		file_exec(g_strdup(command +4));
		back_to_normal_mode(curr_view);
		return;
	}

	/* grep */
	if(!strcmp(arg1, "grep"))
	{
		if(!strcmp(cfg.vi_clone, "vim"))
		{
			g_snprintf(buf, sizeof(buf), "%s \"+ grep %s\"", cfg.vi_command,
				g_strdup(command +5));
			create_zterm(buf, TRUE);
		}
		back_to_normal_mode(curr_view);
		return;
	}

	/* list vi buffers */
	if(!strcmp(arg1, "ls"))
	{
		create_buffer_menu();
		gtk_menu_popup(GTK_MENU(app.buffer_menu), NULL, NULL,
				set_menu_position, NULL, 0, 0);
		back_to_normal_mode(curr_view);
		return;
	}

	/* split */
	if(!strcmp(arg1, "sb"))
	{
		if(arguments > 1)
		{
			command+= 3;
			if(*command == '/')
			{
				change_dir(other_view, command);
				gtk_widget_grab_focus(curr_view->clist);
			}
			else if(*command == '~')
			{
				command++;
				g_snprintf(buf, sizeof(buf), "%s%s", getenv("HOME"), command);
				change_dir(other_view, buf);
				gtk_widget_grab_focus(curr_view->clist);
			}
			else
			{
				g_snprintf(buf, sizeof(buf), "%s/%s", curr_view->dir, command);
				change_dir(other_view, buf);
				gtk_widget_grab_focus(curr_view->clist);
			}
		}
		else
		{
			change_dir(other_view, curr_view->dir);
			gtk_widget_grab_focus(curr_view->clist);
		}
		back_to_normal_mode(other_view);
		return;
	}

	/* change directory */
	if(!strcmp(arg1, "cd"))
	{
		if(arguments > 1)
		{
			command+=3;
			
			if(*command == '/')
			{
				change_dir(curr_view, command);
			}
			else if(*command == '~')
			{
				command++;
				g_snprintf(buf, sizeof(buf), "%s%s", getenv("HOME"), command);
				change_dir(curr_view, buf);
			}
			else
			{
				g_snprintf(buf, sizeof(buf), "%s/%s", curr_view->dir, command);
				change_dir(curr_view, buf);
			}
		}
		else
		{
			change_dir(curr_view, getenv("HOME"));
		}
		back_to_normal_mode(curr_view);
		return;
	}

	/* remove user command from menu */
	if(!strcmp(arg1, "delc"))
	{
		create_delc_menu();
		gtk_menu_popup(GTK_MENU(app.delc_menu), NULL, NULL,
				set_menu_position, NULL, 0, 0);
		back_to_normal_mode(curr_view);
		return;
	}

	/* goto first row :0 */
	if(!strcmp(arg1, "0"))
	{
		goto_row(curr_view->clist, 0);
	}

	/* quit if no terminals are open */
	if((!strcmp(arg1, "q")) | (!strcmp(arg1, "x")))
	{
		if(!cfg.term_count)
			quit_cb(NULL);
	}

	/* quit even if there are terminals open */
	if(!strcmp(arg1, "q!"))
		quit_cb(NULL);



	/* help text */
	if(!strcmp(arg1, "h"))
	{
		g_snprintf(buf, sizeof(buf), "%s %s/vide%s.txt",
				cfg.vi_command, cfg.config_dir, VERSION);
		create_zterm(buf, TRUE);
	}

	/* delete file */
	if(!strcmp(arg1, "d") || !strcmp(arg1, "delete"))
	{
		g_snprintf(buf, sizeof(buf), "mv -f %s %s/", ex_command->str, cfg.trash);
	pipe_and_capture_output_threaded(buf);
	focus_on_row(curr_view,
			curr_view->row   < GTK_CLIST(curr_view->clist)->rows
			? curr_view->row 
			: curr_view->row -1 );
	g_string_free(ex_command, TRUE);
	}

	/* copy file */
	if(!strcmp(arg1, "co") || !strcmp(arg1, "copy"))
	{
		g_snprintf(buf, sizeof(buf), "cp -R %s %s/", ex_command->str, other_view->dir);
		pipe_and_capture_output_threaded(buf);
		g_string_free(ex_command, TRUE);
	}

	/* move file */
	if(!strcmp(arg1, "m") || !strcmp(arg1, "move"))
	{
		g_snprintf(buf, sizeof(buf), "mv  %s %s/", ex_command->str,
				other_view->dir);
		pipe_and_capture_output_threaded(buf);
		g_string_free(ex_command, TRUE);
		if(curr_view->row < GTK_CLIST(curr_view->clist)->rows)
		{
			focus_on_row(curr_view, curr_view->row);
		}
		else
		{
			focus_on_row(curr_view, GTK_CLIST(curr_view->clist)->rows -1);
		}
	}
	if(!g_strncasecmp(command, "!!", 2))
	{
		command +=2;
		g_snprintf(buf, sizeof(buf), "pauseme %s", command);
		create_zterm(buf, TRUE);
		back_to_normal_mode(curr_view);
		return;
	}

	/* shell commands */
	if(*command == '!')
	{
		command++;
		create_zterm(command, TRUE);
		back_to_normal_mode(curr_view);
		return;
	}


	/* search / */
	if(*command == '/')
	{
		g_snprintf(pat.pattern, sizeof(pat.pattern), "%s", command);
		find_pattern(curr_view);
	}

	/* open a shell */ 
	if(!strcmp(arg1, "sh") || !strcmp(arg1, "shell"))
	{
		create_zterm("sh", TRUE);
	}

	/* show bookmarks */
	if(!strcmp(arg1, "marks"))
	{
		gtk_signal_emit_by_name(GTK_OBJECT(app.bookmark_menu_item),
				"activate-item");
	}

	/* run vide as root */
	if(!strcmp(arg1, "su"))
	{
		/* change to allow su username? */
		gchar root_command[NAME_MAX];
		g_snprintf(root_command, sizeof(root_command), "su root -c vide");
		create_zterm(root_command, TRUE);
	}

	/* history */
	if(!strcmp(arg1, "his") || !strcmp(arg1, "history"))
		create_history_menu(curr_view);

	/* load file into vi */
	if(!strcmp(arg1, "e"))
	{
		g_snprintf(buf, sizeof(buf), "%s %s", cfg.vi_command, ex_command->str); 
		create_zterm(buf, TRUE);
		g_string_free(ex_command, TRUE);
		back_to_normal_mode(curr_view);
		return;
	}

	/* one window */
	if(!strcmp(arg1, "on") || !strcmp(arg1, "only"))
	{
		if(GTK_WIDGET_HAS_FOCUS(GTK_WIDGET(app.left_view.clist)))
		{
			gtk_widget_hide(GTK_WIDGET(app.right_view_box));
			gtk_widget_hide(GTK_WIDGET(app.sw));
		}
		else 
		{
			gtk_widget_hide(GTK_WIDGET(app.left_view_box));
			gtk_widget_hide(GTK_WIDGET(app.sw));
		}
	}

	/* show two filelists */
	if(!strcmp(arg1, "new"))
	{
		if(GTK_WIDGET_HAS_FOCUS(GTK_WIDGET(app.left_view.clist)))
		{
			gtk_widget_show(GTK_WIDGET(app.right_view_box));
			gtk_widget_show(GTK_WIDGET(app.sw));
		}
		else
		{
			gtk_widget_show(GTK_WIDGET(app.left_view_box));
			gtk_widget_show(GTK_WIDGET(app.sw));
		}
	}

	/*set filter */
	if(!g_strncasecmp(command, "set filter", 10))
	{
		if(!strcmp("invert",  g_strndup(command +11, 6)))
		{
			command += 18;
			strncpy(curr_view->filename_filter.pattern, command,
					sizeof(curr_view->filename_filter.pattern));
			curr_view->filename_filter.active = TRUE;
			curr_view->filename_filter.invert_mask = TRUE;
		}
		else
		{
			command += 11;
			strncpy(curr_view->filename_filter.pattern, command,
					sizeof(curr_view->filename_filter.pattern));
			curr_view->filename_filter.active = TRUE;
			curr_view->filename_filter.invert_mask = FALSE;
		}

		load_dir_list(curr_view);
	}



	/*filter */
	if(!g_strncasecmp(command, "fil", 3) |
			(!g_strncasecmp(command, "filter", 6)))
	{
		curr_view->filename_filter.active = TRUE;
		load_dir_list(curr_view);
	}

	/*nofilter */
	if(!g_strncasecmp(command, "nofil", 5) |
			!g_strncasecmp(command, "nofilter", 8))
	{
		curr_view->filename_filter.active = FALSE;
		load_dir_list(curr_view);
	}

	/*invert*/
	if(!g_strncasecmp(command, "in", 2) || !g_strncasecmp(command, "invert", 6))
	{
		if(curr_view->filename_filter.invert_mask)
			 curr_view->filename_filter.invert_mask = FALSE;
		else
			 curr_view->filename_filter.invert_mask = TRUE;

		load_dir_list(curr_view);
	}
	/* shows current working directory on status bar submitted by Sergei Gnezdov */
	if((!strcmp(arg1, "pw")) | (!strcmp(arg1, "pwd")))
	{
		status_bar_message(curr_view->dir);
		reset_signals();
		return;
	}


	/* sp b and vs */
	if((!g_strncasecmp(command, "sp", 2)) | (!g_strncasecmp(command,"vs", 2))
			| (!g_strncasecmp(command, "b", 1)))
	{

		ZvtTerm *term;
		int sp = 0;
		int b = 0;
		if(!g_strncasecmp(command, "sp", 2))
			sp = 1;
		if(!g_strncasecmp(command, "b", 1))
			b = 1;

		if(b)
			command++;
		else
			command+=2;

		chomp(command);

		while(isspace(*command))
		{
			command++;
		}

		if(isalpha(*command))
		{
			back_to_normal_mode(curr_view);
			return;
		}

		if(isdigit(*command))
		{
			gint number = atoi(command) - 1;

			/* selected the vide notebook page */
			if(number == 0)
			{
				back_to_normal_mode(curr_view);
				status_bar_message("Page 1 is not an editor");
				return;
			}

			if(number < 0)
				number = 9;

			if((term = get_focus_zvt(GTK_NOTEBOOK(app.notebook), number)))
			{

				if(GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(term))))
				{
					g_snprintf(buf, sizeof(buf), "%c\r", GDK_Escape);
					write_to_term(term, buf);
					if(sp)
					{
						/* I don't know how to split a window in nvi */
						if(!strcmp("vi", cfg.vi_clone))
						{
							back_to_normal_mode(curr_view);
							return;
						}
						else if(!strcmp("vile", cfg.vi_clone))
						{
							g_snprintf(buf, sizeof(buf), ":split-current-window\r\n");
							write_to_term(term, buf);
							g_snprintf(buf, sizeof(buf), ":e %s/%s\r\n", 
									curr_view->dir, file);
							write_to_term(term, buf);
							gtk_notebook_set_page(GTK_NOTEBOOK(app.notebook), number);
							gtk_widget_grab_focus(GTK_WIDGET(term));
							back_to_normal_mode(curr_view);
							return;
						}
						else
						{
							g_snprintf(buf, sizeof(buf), ":sp %s/%s\r\n",
								curr_view->dir, file);
						}
					}
					else if(b)
					{
						g_snprintf(buf, sizeof(buf), ":e %s/%s\r\n",
								curr_view->dir, file);
					}
					else
					{
						if(!strcmp("vim", cfg.vi_clone))
						{
							g_snprintf(buf, sizeof(buf), ":vs %s/%s\r\n",
								curr_view->dir, file);
						}
						else
						{
							back_to_normal_mode(curr_view);
							return;
						}
					}
					write_to_term(term, buf);
					gtk_notebook_set_page(GTK_NOTEBOOK(app.notebook), number);
					gtk_widget_grab_focus(GTK_WIDGET(term));
				}
				else
				{
					back_to_normal_mode(curr_view);
					status_bar_message("That terminal is not an editor");
					return;
				}
			}
		}
		else
		{
			gint page = 1;
			while((term = get_focus_zvt(GTK_NOTEBOOK(app.notebook), page)))
			{
				if(term == cfg.current_term)
					break;

				page++;
			}
			if(GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(term))))
			{
				g_snprintf(buf, sizeof(buf), "%d\r", GDK_Escape);
				write_to_term(term, buf);

				if(sp)
				{
					if(!strcmp("vi", cfg.vi_clone))
					{
						back_to_normal_mode(curr_view);
						return;
					}
					else if(!strcmp("vile", cfg.vi_clone))
					{
						write_to_term(term, ":split-current-window\r\n");
						g_snprintf(buf, sizeof(buf), ":e %s/%s\r\n",
								curr_view->dir, file);
						write_to_term(term, buf);
						gtk_notebook_set_page(GTK_NOTEBOOK(app.notebook), page);
						gtk_widget_grab_focus(GTK_WIDGET(term));
						back_to_normal_mode(curr_view);
						return;
					}
					else
					{
						g_snprintf(buf, sizeof(buf), ":sp %s/%s\r",
							curr_view->dir, file);
					}
				}
				else if(b)
				{
					g_snprintf(buf, sizeof(buf), ":e %s/%s\r",
							curr_view->dir, file);
				}
				else
				{
					if(!strcmp("vim", cfg.vi_clone))
					{
						g_snprintf(buf, sizeof(buf), ":vs %s/%s\r",
							curr_view->dir, file);
					}
					else
					{
						back_to_normal_mode(curr_view);
						return;
					}
				}
	
				write_to_term(term, buf);
				gtk_notebook_set_page(GTK_NOTEBOOK(app.notebook), page);
				gtk_widget_grab_focus(GTK_WIDGET(term));
			}
			else
			{
				back_to_normal_mode(curr_view);
				status_bar_message("That terminal is not an editor");
				return;
			}
		}
	}

	back_to_normal_mode(curr_view);	
}

static void
command_key_cb(GtkWidget *widget, GdkEventKey *event, FileView *view)
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
		gchar prog[1024];
		com.index = '\0';
		g_snprintf(prog, sizeof(prog), "%s", com.buffer);
		event->keyval = 0;

		execute_command(prog);
		return;
	}
	else if (event->keyval == GDK_BackSpace)
	{
		if(com.index > 0)
		{
			com.index--;
			com.buffer[com.index] = '\0';
			status_bar_message(com.buffer);
		}
	}
	else if((event->keyval < 0x100) && (event->state == 0 
				|| event->state & GDK_SHIFT_MASK))
	{
		com.buffer[com.index] = event->keyval;
		if(com.index < 1024)
			com.index++;
		status_bar_message(com.buffer);
	}
	else
		event->keyval = 0;
}


void
command_mode(FileView *view)
{
	guint key_press_signal;
	com.index = 0;


	key_press_signal = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(view->clist),  "signal_key_id"));
	gtk_signal_disconnect(GTK_OBJECT(view->clist), key_press_signal);

	key_press_signal = gtk_signal_connect(GTK_OBJECT(view->clist),
			"key_press_event", GTK_SIGNAL_FUNC(command_key_cb), view);
	gtk_object_set_data(GTK_OBJECT(view->clist), "signal_key_id",
			GUINT_TO_POINTER(key_press_signal));
	
}
