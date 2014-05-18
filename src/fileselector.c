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

#include "vide.h"

struct sentinel {
	gint g;
	gint action;
	gint page;
}sentinel;

void
reset_to_normal(void)
{
	guint key_press_signal;

	cfg.fileselector = FALSE;

	if(curr_view->clist == app.left_view.clist)
	{
		gtk_widget_show(GTK_WIDGET(app.right_view_box));
		gtk_widget_show(GTK_WIDGET(app.sw));
	}
	else
	{
		gtk_widget_show(GTK_WIDGET(app.left_view_box));
		gtk_widget_show(GTK_WIDGET(app.sw));
	}

	key_press_signal = GPOINTER_TO_INT(gtk_object_get_data
			(GTK_OBJECT(curr_view->clist),  "signal_key_id"));
	gtk_signal_disconnect(GTK_OBJECT(curr_view->clist), key_press_signal);

	key_press_signal = gtk_signal_connect(GTK_OBJECT(curr_view->clist),
			"key_press_event", GTK_SIGNAL_FUNC(file_list_key_press_cb),
			curr_view);
	gtk_object_set_data(GTK_OBJECT(curr_view->clist), "signal_key_id",
			GUINT_TO_POINTER(key_press_signal));


	gtk_notebook_set_page(GTK_NOTEBOOK(app.notebook),
			sentinel.page);

}

static void
fileselect_commands()
{

}

static void
handle_selection(GdkEventKey *event)
{
	FileInfo *info;


  if (curr_view->tagged != NULL)
    info = curr_view->tagged->data;
  else
    info = gtk_clist_get_row_data (GTK_CLIST (curr_view->clist),
				curr_view->row);
  g_return_if_fail (info != NULL);


  if (is_dir (info))
    {
      gchar path[PATH_MAX];


      if (strcmp (info->filename, "../") == 0)
			{
				change_dir(curr_view, "..");
	  		return;
			}
      g_snprintf (path, sizeof (path), "%s/%s", curr_view->dir, info->filename);
    change_dir (curr_view, path);
      return;
    }
	else
	{
		switch(sentinel.action)
		{
			gchar buf[PATH_MAX];

			case 0: /* load a file into the current vi buffer :e */
				{
					event->keyval = 0;
					g_snprintf(buf, sizeof(buf), "%d\r", GDK_Escape);
					write_to_term(cfg.current_term, buf);

					g_snprintf(buf, sizeof(buf), ":e %s/%s\r\n", curr_view->dir,
				 			info->filename);
					write_to_term(cfg.current_term, buf);
					event->keyval = 0;
					reset_to_normal();
				}
				break;
			case 1:/* split vi buffer and load file */ 
				{
					g_snprintf(buf, sizeof(buf), "%d\r", GDK_Escape);
					write_to_term(cfg.current_term, buf);

					if(!strcmp("vile", cfg.vi_clone))
					{
						g_snprintf(buf, sizeof(buf), ":split-current-window\r\n");
						write_to_term(cfg.current_term, buf);
						g_snprintf(buf, sizeof(buf), ":e %s/%s\r\n", 
								curr_view->dir, info->filename);
						write_to_term(cfg.current_term, buf);
					}
					else
					{
						g_snprintf(buf, sizeof(buf), ":sp %s/%s\r\n",
							curr_view->dir, info->filename);
						write_to_term(cfg.current_term, buf);
					}
					event->keyval = 0;
					reset_to_normal();
				}
				break;
			case 2:/* vertically split vi bufer and load file */
				{
					g_snprintf(buf, sizeof(buf), "%d\r", GDK_Escape);
					write_to_term(cfg.current_term, buf);

					g_snprintf(buf, sizeof(buf), ":vs %s/%s\r\n", curr_view->dir,
							info->filename);
					write_to_term(cfg.current_term, buf);
					event->keyval = 0;
					reset_to_normal();
				}
				break;
			case 3:/* open new terminal with vi and file */
				{
					g_snprintf(buf, sizeof(buf), "%s %s/%s", cfg.vi_command,
							curr_view->dir, info->filename);

					/* reset the notebook page first otherwise it will create the
					 * new term on the filemanager page.
					 */
					gtk_notebook_set_page(GTK_NOTEBOOK(app.notebook), sentinel.page);
					create_zterm(buf, FALSE);
					event->keyval = 0;
					reset_to_normal();
				}
				break;
			case 4:/* use vimdiff program in new terminal */
				{
				}
				break;

			default:
				event->keyval = 0;
				reset_to_normal();
				break;
		}
	}
}

void
fileselector_key_cb(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  /* Ctrl+<letter> Shortcuts */
  if (event->state & GDK_CONTROL_MASK)
    {

      switch (event->keyval)
			{
				case 'f':
	  			event->state = 0;
	  			event->keyval = GDK_Page_Down;
	  			break;
				case 'b':
	  			event->state = 0;
	  			event->keyval = GDK_Page_Up;
	  			break;
				case 'c':
					{
						event->keyval = 0;
						reset_to_normal();
					}
	  			break;
		/* Ctrl g shows file properties on status bar - patch by Sergei Gnezdov */
				case 'g':
					show_full_file_properties();
					break;

				default:
	  			break;
			}


      return;
    }

  /* Alt+<letter> Shortcuts */
  if (event->state & GDK_MOD1_MASK)
    {
      switch (event->keyval)
			{
				case 'B':
				case 'b':
	  			gtk_signal_emit_by_name (GTK_OBJECT (app.bookmark_menu_item),
				   	"activate-item");
	  			break;

				case 'T':
				case 't':
					gtk_signal_emit_by_name(GTK_OBJECT(app.tools_menu_item),
							"activate-item");
					break;
				case 'V':
				case 'v':
	  			gtk_signal_emit_by_name (GTK_OBJECT (app.vide_menu_item),
				   	"activate-item");
	  			break;
				case 'O':
				case 'o':
	  			gtk_signal_emit_by_name (GTK_OBJECT (app.options_menu_item),
				   	"activate-item");
	  			break;
				case 'H':
				case 'h':
	  			gtk_signal_emit_by_name (GTK_OBJECT (app.help_menu_item),
				   	"activate-item");
	  			break;
				default:
	  			break;
			}
      return;
   }

  /* Other Shortcuts */
  switch (event->keyval)
    {
			/* Ignore tab and space otherwise Gtk+ will change which 
			 * widget is focused.
			 */
			case GDK_Tab:
			case GDK_space:
				event->keyval = 0;
				break;

    	case GDK_Return:
				handle_selection(event);
				break;
			default:
      	break;
    }

  /* code to mimic vi key bindings */
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
				case 'G':
	  			goto_row (widget, GTK_CLIST (widget)->rows - 1);
	  			break;
				case 'g':
					{
						if(sentinel.g)
						{
							sentinel.g = 0;
	  					goto_row (widget, 0);
	  					break;
						}
						else
						{
							sentinel.g = 1;
							break;
						}
					}
					
				case 'l':
	  			event->keyval = GDK_Right;
	  			break;
				case 'u':
	  			change_dir(curr_view, "..");
	  			break;
				case 'h':
	  			event->keyval = GDK_Left;
					return;
	  			break;
				case 'p':
					create_filetype_popup();
					gtk_menu_popup(GTK_MENU(app.filetype_popup), NULL, NULL,
				 	set_menu_position, NULL, event->type, event->time);
	  			break;
				case 'P':
					create_programs_menu();
					gtk_menu_popup(GTK_MENU(app.programs_menu), NULL, NULL,
								set_menu_position, NULL, event->type, event->time);
					break;
				case 't':
					toggle_tag_cb();
					break;
				case 'm':
					event->keyval = 0;
					set_mark(curr_view);
					break;
				case GDK_apostrophe: 
					event->keyval = 0;
					goto_mark(curr_view);
					 break;
				case 'n':
					if(curr_view->glob == NULL)
						break;
					find_next(curr_view);
					break;
				case 'N':
					if(curr_view->glob == NULL)
						break;
					find_prev(curr_view);
					break;
				case '/':
					{
	  			search_mode(curr_view);
					}
	  			break;
				case ':':
					fileselect_commands(curr_view);
					break;
				default:
	  			break;
		}
  }
	return;
}

void
create_fileselector(GtkWidget *widget, gpointer action)
{
	guint key_press_signal;

	sentinel.action = GPOINTER_TO_INT(action);
	sentinel.page = gtk_notebook_get_current_page(GTK_NOTEBOOK(app.notebook));

	cfg.fileselector = TRUE;


	if(curr_view->clist == app.left_view.clist)
	{
		gtk_widget_hide(GTK_WIDGET(app.right_view_box));
		gtk_widget_hide(GTK_WIDGET(app.sw));
	}
	else
	{
		gtk_widget_hide(GTK_WIDGET(app.left_view_box));
		gtk_widget_hide(GTK_WIDGET(app.sw));
	}


	key_press_signal = GPOINTER_TO_INT(gtk_object_get_data(
				GTK_OBJECT(curr_view->clist),  "signal_key_id"));

	gtk_signal_disconnect(GTK_OBJECT(curr_view->clist), key_press_signal);

	key_press_signal = gtk_signal_connect(GTK_OBJECT(curr_view->clist),
			"key_press_event", GTK_SIGNAL_FUNC(fileselector_key_cb), NULL);
	
	gtk_object_set_data(GTK_OBJECT(curr_view->clist), "signal_key_id",
			GUINT_TO_POINTER(key_press_signal));

	gtk_notebook_set_page(GTK_NOTEBOOK(app.notebook), 0);

}
