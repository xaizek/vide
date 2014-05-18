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


#include <glob.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <time.h>
#include "vide.h"
#include <gdk/gdkx.h>
/* #include <gnome.h> */
#include "icon_dirparent.xpm"

struct sentinel {
	gint d;
	gint g;
}sentinel;

static GtkTargetEntry target_table[] = {
  {"text/plain", 0, 0}
};

static guint n_targets = sizeof (target_table) / sizeof (target_table[0]);

void
notebook_cb(GtkNotebook *notebook, GtkNotebookPage *page,
		gint page_num, gpointer data)
{

	if(page_num > 0)
	{
		GtkWidget *label;
		GdkColor color = TAB_COLOR;
		GtkStyle *defstyle;
		GtkStyle *style = gtk_style_new();
		ZvtTerm *term;
		gint a = 1;
		gchar *title;
		gchar *text;
		gchar buf[128];


		defstyle = gtk_widget_get_default_style();
		style = gtk_style_copy(defstyle);
		style->fg[0] = color;

		/* reset the page numbers in case a page was deleted when a
		 * terminal was detached.
		 */
		while(gtk_notebook_get_nth_page(GTK_NOTEBOOK(app.notebook), a))
		{
			label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(app.notebook),
					gtk_notebook_get_nth_page(GTK_NOTEBOOK(app.notebook), a));
				gtk_label_get(GTK_LABEL(label), &text);
			title = strstr(text, " ") +1;
			if(a +1 == 10)
				g_snprintf(buf, sizeof(buf), "%d %s", 0, title);
			else
				g_snprintf(buf, sizeof(buf), "%d %s", a +1, title);
			gtk_label_set_text(GTK_LABEL(label), buf);
			a++;
		}
		a = 1;

		term = get_nth_zvt(GTK_NOTEBOOK(app.notebook), page_num);
		if(GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(term))))
		{

			/* reset all pages to normal */ 
			while((term = get_nth_zvt(GTK_NOTEBOOK(app.notebook), a)))
			{
				label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(app.notebook),
						gtk_notebook_get_nth_page(GTK_NOTEBOOK(app.notebook), a));
				gtk_label_get(GTK_LABEL(label), &text);
				title = strstr(text, " ") +1;
				if(a +1 == 10)
					g_snprintf(buf, sizeof(buf), "%d %s", 0, title);
				else
					g_snprintf(buf, sizeof(buf), "%d %s", a +1, title);
				gtk_label_set_text(GTK_LABEL(label), buf);

				gtk_widget_set_style(GTK_WIDGET(label), defstyle);
				gtk_object_set_data(GTK_OBJECT(term), "page_number",
						GINT_TO_POINTER(a));
				a++;

			}
			/* highlight current buffer */
			label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(app.notebook),
					gtk_notebook_get_nth_page(GTK_NOTEBOOK(app.notebook), page_num));
			gtk_widget_set_style(GTK_WIDGET(label), style);
			cfg.current_term = get_nth_zvt(GTK_NOTEBOOK(app.notebook), page_num);
		}
	}
	else 
	{
		goto_row(command_view->clist, 0);
 		gtk_widget_grab_focus (curr_view->clist);
		show_file_properties();
		/*

		if(curr_view->row >= GTK_CLIST(curr_view->clist)->rows)
			focus_on_row(curr_view, 0);
		else
		{
			focus_on_row(curr_view, curr_view->row);
		}
 		gtk_widget_grab_focus (curr_view->clist);
		*/
	}
}

static void
toggle_command_style ()
{
  GtkStyle *style;

  style = gtk_style_new ();
  style->bg[GTK_STATE_NORMAL] = COL_COLOR;
  if (!GTK_WIDGET_HAS_FOCUS (GTK_WIDGET (command_view->clist)))
    {
      gtk_widget_set_style (GTK_CLIST (command_view->clist)->column[0].button,
			    style);
    }
  else
    gtk_widget_restore_default_style (GTK_CLIST (command_view->clist)->
				      column[0].button);
}


static void
switch_views ()
{
  gint i;
  gint num_cols;
  GtkStyle *style;
  FileView *temp;

  temp = other_view;
  other_view = curr_view;
  curr_view = temp;

  gtk_signal_emit_by_name (GTK_OBJECT (other_view->clist), "end-selection");
  gtk_clist_unselect_all (GTK_CLIST (other_view->clist));

  style = gtk_style_new ();
  style->bg[GTK_STATE_NORMAL] = COL_COLOR;
  num_cols = GTK_CLIST (curr_view->clist)->columns;
  for (i = 0; i < num_cols; i++)
    {
      gtk_widget_set_style (GTK_CLIST (curr_view->clist)->column[i].button,
			    style);
      gtk_widget_set_style (curr_view->sort_arrows[i], style);
    }
  for (i = 0; i < num_cols; i++)
    {
      gtk_widget_restore_default_style (GTK_CLIST (other_view->clist)->
					column[i].button);
      gtk_widget_restore_default_style (other_view->sort_arrows[i]);
    }

  chdir (curr_view->dir);
}


static gboolean
command_row_cb (FileView * command_view)
{
  GList *tmp;
	gchar buf[1024];

  for (tmp = cfg.user_commands; tmp != NULL; tmp = tmp->next)
    {
      gchar *command_text;
      gchar *file_name;
      GString *expanded_com;
      UserCommand *command = tmp->data;
      gtk_clist_get_text (GTK_CLIST (command_view->clist),
			  command_view->row, 0, &command_text);
      gtk_clist_get_text (GTK_CLIST (curr_view->clist),
			  curr_view->row, 0, &file_name);
      if (!strcmp (command_text, command->name))
				{
	  			expanded_com = expand_macros (command->action);
					if(expanded_com == NULL)
					{
						gtk_widget_grab_focus(curr_view->clist);
						toggle_command_style();
						goto_row(command_view->clist, 0);
						g_string_free(expanded_com, TRUE);
					}
					else if(!g_strncasecmp("!", expanded_com->str, 1))
					{
						g_snprintf(buf, sizeof(buf), ":%s", expanded_com->str);
						execute_command(buf);
						g_string_free(expanded_com, TRUE);
						return TRUE;
					}
					else
					{
						pipe_and_capture_output_threaded(expanded_com->str);
						g_string_free(expanded_com, TRUE);
					}
				}
    }
	return FALSE;
}

void
load_user_commands (void)
{
  GList *tmp;


  for (tmp = cfg.user_commands; tmp != NULL; tmp = tmp->next)
    {
      gchar *name_buf[NAME_MAX];
      UserCommand *command = tmp->data;

      *name_buf = command->name;
      gtk_clist_append (GTK_CLIST (command_view->clist), name_buf);

    }
}

void
mouse_command_cb(GtkWidget *clist, gint row, gint col, GdkEvent *event, FileView *view)
{
  	if (event)
    {
      switch (event->type)
			{
				case GDK_2BUTTON_PRESS:
	  				command_row_cb(command_view);
						gtk_widget_grab_focus(curr_view->clist);
						goto_row(command_view->clist, 0);

	  				return;
	  				break;

				default:
	  			break;
			}
		}
}

void
command_select_row_cb (GtkWidget * clist,
		       gint row, gint col, GdkEvent * event, FileView * view)
{
  gchar status_text[128];
  view->row = row;
		
  if (command_view->row == 0)
    {
      g_snprintf (status_text, sizeof (status_text), "No Commands Selected");
      gtk_label_set_text (GTK_LABEL (app.status_bar), status_text);
    } 
  else
    {
      GList *tmp;
      for (tmp = cfg.user_commands; tmp != NULL; tmp = tmp->next)
				{
	  			gchar *command_text;
	  			gchar *file_name;
					GString *expanded_com;
	  			UserCommand *command = tmp->data;
	  			gtk_clist_get_text (GTK_CLIST (command_view->clist),
			      command_view->row, 0, &command_text);
	  			gtk_clist_get_text (GTK_CLIST (curr_view->clist),
			      curr_view->row, 0, &file_name);
	  			if (!strcmp (command_text, command->name))
	    		{
	      		expanded_com = list_macros (command->action);

	      		g_snprintf (status_text, sizeof (status_text),
		  			"%s", expanded_com->str);
	      		gtk_label_set_text (GTK_LABEL (app.status_bar), status_text);
						g_string_free(expanded_com, TRUE);
	    		}
				}
    }
}

static void
select_row_cb (GtkWidget * clist,
	       gint row, gint col, GdkEvent * event, FileView * view)
{
  view->row = row;

  if (curr_view != view)
    switch_views ();

	/*
  g_snprintf (status_text, sizeof (status_text),
	      "%d of %d files selected",
	      g_list_length (GTK_CLIST (view->clist)->selection),
	      GTK_CLIST (view->clist)->rows);
  gtk_label_set_text (GTK_LABEL (app.status_bar), status_text);
	*/

	show_file_properties();

  	if (event)
    {
      switch (event->type)
			{
				case GDK_2BUTTON_PRESS:
	  				handle_file (view);
	  				return;
	  				break;

					default:
	  			break;
			}
    } 
}


void
window_key_press_cb (GtkWidget * widget, GdkEventKey * event)
{
	if(event->state & GDK_CONTROL_MASK)
	{
		switch(event->keyval)
		{
			case 'c':
				{
					sentinel.g = 0;
					sentinel.d = 0;
					break;
				}
			default:
				break;
		}

	}

  if (event->state & GDK_MOD1_MASK)
	{
		switch(event->keyval)
		{
			case 'q':
				quit_cb(NULL);
				break;
			default:
				break;
		}
	}
}

/* This puts the popup menu over the current clist */
void
set_menu_position(GtkMenu *menu, gint *x, gint *y, gpointer data)
{
	gdk_window_get_origin((curr_view->clist)->window, x, y);
	*x += 2;
	*y += 2;
}

void
mouse_cb(GtkWidget *widget, GdkEventButton *event, FileView *view)
{
	if(view == command_view)
		return;

	if(event->button == 3)
	{
		int x, y, row, column;
		GdkModifierType state;
		/*
		 * there must be an easier way to get the right mouse click to select
		 * a row
		 */

		gdk_window_get_pointer(event->window, &x, &y, &state);
		gtk_clist_get_selection_info(GTK_CLIST(view->clist), x, y, &row, &column);
		gtk_clist_unselect_all(GTK_CLIST(view->clist));
		gtk_clist_select_row(GTK_CLIST(view->clist), row, column);

		create_filetype_popup();
		gtk_menu_popup(GTK_MENU(app.filetype_popup), NULL, NULL,
		set_menu_position, NULL, event->type, event->time);
	}
}


void
file_list_key_press_cb (GtkWidget * widget,
			GdkEventKey * event, FileView * view)
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
					sentinel.d = 0;
					sentinel.g = 0;
	  			break;
		/* Ctrl g shows file properties on status bar - patch by Sergei Gnezdov */
				case 'g':
					show_full_file_properties();
					break;

				default:
	  			break;
			}

		if(event->keyval >= '0' && event->keyval <= '9')
		{
			ZvtTerm *focus_term;
			int page = event->keyval;

			if(page == '0')
			{
				page = 9;
			}
			else
			{
				page = page -49;
			}

			gtk_notebook_set_page(GTK_NOTEBOOK(app.notebook), page);
			focus_term = get_focus_zvt(GTK_NOTEBOOK(app.notebook), page);
			gtk_widget_grab_focus(GTK_WIDGET(focus_term));
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
				case 'h':
	  			gtk_signal_emit_by_name (GTK_OBJECT (app.help_menu_item),
				   	"activate-item");
	  			break;
				case 'C':
				case 'c':
					gtk_signal_emit_by_name(GTK_OBJECT(app.command_menu_item),
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
    case GDK_Return:
      if (GTK_WIDGET_HAS_FOCUS (command_view->clist))
			{
				gboolean uses_zterm;

	  		gtk_signal_emit_stop_by_name (GTK_OBJECT (command_view->clist),
						"key_press_event");
				gtk_widget_grab_focus(curr_view->clist);
				gtk_widget_restore_default_style(GTK_CLIST
						(command_view->clist)->column[0].button);
	
				uses_zterm = command_row_cb(command_view);

				if(!uses_zterm)
				{
					goto_row(command_view->clist, 0);
					show_file_properties();
				}
				return;
				break;
			}
      else
			{
				handle_file(view);
	  		break;
			}

    case GDK_Tab:
    case GDK_space:
      if (GTK_WIDGET_HAS_FOCUS (command_view->clist))
			{
	  		toggle_command_style ();
	  		goto_row (command_view->clist, 0);

				if(other_view->row >= GTK_CLIST(other_view->clist)->rows)
				{
					focus_on_row(other_view, 0);
				}
				else
					focus_on_row(other_view, other_view->row);
	  		gtk_widget_grab_focus (curr_view->clist);

	  		gtk_signal_emit_stop_by_name (GTK_OBJECT (command_view->clist),
						"key_press_event");
	  		gtk_signal_emit_stop_by_name (GTK_OBJECT (app.main_window),
						"key_press_event");
			}
      else
			{
	  		toggle_command_style ();
	  		goto_row (command_view->clist, 0);
	  		gtk_widget_grab_focus (command_view->clist);
	  		gtk_signal_emit_stop_by_name (GTK_OBJECT (curr_view->clist),
						"key_press_event");
	  		gtk_signal_emit_stop_by_name (GTK_OBJECT (app.main_window),
						"key_press_event");
			}
      return;
      break;

    case GDK_Insert:
      toggle_tag_cb ();
      break;

    default:
      break;
    }

  /* code to mimic vi key bindings */
  if ((event->keyval < 0x100)
      && (event->state == 0 || event->state & GDK_SHIFT_MASK
	  || event->state & GDK_MOD2_MASK))
    {
      /* code to mimic vi key bindings 
			 *
       * j =  move up in the list
       * k = move down the list
       * l = move right in the list
       * h = move left in the list
       * u = Up directory
       * gg = goto top of list
       * G = goto bottom of list
       */
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
					
				case 'd':
					{
						if(GTK_WIDGET_HAS_FOCUS(command_view->clist))
						{
							sentinel.d = 0;
							break;
						}
						if(sentinel.d)
						{
							gchar buf[PATH_MAX];
							gchar *file;
							gtk_clist_get_text(GTK_CLIST(view->clist), view->row,
									0, &file);
							g_snprintf(buf, sizeof(buf), "mv -f \"%s\" %s/", file, cfg.trash);
							pipe_and_capture_output_threaded(buf);
							gtk_clist_remove(GTK_CLIST(view->clist), view->row);
							focus_on_row(view,
									view->row < GTK_CLIST(view->clist)->rows
									? view->row
									: view->row -1);
							sentinel.d = 0;
							break;
						}
						else
						{
							sentinel.d = 1;
							break;
						}
					}
				case 'l':
	  			event->keyval = GDK_Right;
	  			break;
				case 'u':
					if(GTK_WIDGET_HAS_FOCUS(command_view->clist))
						break;
	  			change_dir(view, "..");
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
				case 'v':
					/* visual mode to select multiple files */
					break;
				case 'm':
					if (GTK_WIDGET_HAS_FOCUS(command_view->clist))
						break;
					event->keyval = 0;
					set_mark(view);
					break;
				case GDK_apostrophe: 
					if(GTK_WIDGET_HAS_FOCUS(command_view->clist))
						break;
					event->keyval = 0;
					goto_mark(view);
					 break;
				case 'n':
					if(GTK_WIDGET_HAS_FOCUS(command_view->clist))
						 break;
					if(view->glob == NULL)
						break;
					find_next(view);
					break;
				case 'N':
					if(GTK_WIDGET_HAS_FOCUS(command_view->clist))
						break;
					if(view->glob == NULL)
						break;
					find_prev(view);
					break;
				case ':':
					if(GTK_WIDGET_HAS_FOCUS(command_view->clist))
						break;
					else
					{
	  				command_mode(view);
						break;
					}
				case '/':
					if(GTK_WIDGET_HAS_FOCUS(command_view->clist))
						break;
					else
					{
	  				search_mode(view);
	  				break;
					}
				default:
	  			break;
		}
  }
}


/*
 * STATUS MESSAGE FUNCTIONS
 */

void
status_bar_message (gchar * msg)
{
  gtk_label_set_text (GTK_LABEL (app.status_bar), msg);
}

void
status_errno ()
{
	status_bar_message(g_strerror(errno));
}

/*
 * Up Directory Button Callback
 */

static void
updir_click_cb(GtkWidget *widget, GdkEventButton *event, FileView *view)
{
  if (event->button == 1)
  {
    gchar path[PATH_MAX];
    g_snprintf(path, sizeof(path), "%s/..", view->dir);
    change_dir(view, path);
  }
  else if (event->button == 3)
  {
    gchar *home;
    if ((home = getenv("HOME")) != NULL)
      change_dir(view, home);
  }
}


static void
column_button_cb(GtkWidget *widget, gint col, FileView *view)
{
	GtkSortType direction;

	if(GTK_CLIST(view->clist)->sort_column == col)
		direction = (GTK_CLIST(view->clist)->sort_type == GTK_SORT_ASCENDING
				? GTK_SORT_DESCENDING : GTK_SORT_ASCENDING);
	else
		direction = GTK_CLIST(view->clist)->sort_type;
	
	gtk_clist_freeze(GTK_CLIST(view->clist));
	gtk_clist_unselect_all(GTK_CLIST(view->clist));
	sort_list(view, all_columns[col].sort_func, direction, col);
	gtk_widget_grab_focus(GTK_WIDGET(view->clist));
	gtk_clist_select_row(GTK_CLIST(view->clist), 0, 0);
	gtk_clist_thaw(GTK_CLIST(view->clist));
}

/*
 * WIDGET BUILDERS
 */

static GtkWidget *
create_file_view (FileView * view)
{
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkStyle *style;
  GtkWidget *sw;
	guint key_press_signal;
	gint i;
	sentinel.g = 0;
	sentinel.d = 0;

  /* Top Pane */
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);


  style = gtk_widget_get_style (app.main_window);

  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  sw = gtk_scrolled_window_new (NULL, NULL);
	if(cfg.use_scrollbars)
	{
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(sw),
				GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
	}
	else
	{
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_NEVER, GTK_POLICY_NEVER);
	}
  gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
  gtk_widget_show (sw);

  /* File List */
  view->clist = gtk_clist_new (MAX_COLUMNS);
  for (i = 0; i < MAX_COLUMNS; i++)
    {
      GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
      GtkWidget *label = gtk_label_new (all_columns[i].title);

      view->sort_arrows[i] = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_IN);

			/* put up directory button in first column if using scrollbars */ 
			if ( i == 0)
			{
				/* start as blank label will be reset as soon as directory loads */
				view->dir_label = gtk_label_new(" ");
				gtk_box_pack_start(GTK_BOX(hbox), view->dir_label, FALSE, TRUE, 0);
				gtk_widget_show(view->dir_label);

				if(cfg.use_scrollbars)
				{
					GtkTooltips *tooltips;
					GtkWidget *pixmapwid;
					GdkPixmap *pixmap;
					GdkBitmap *mask;

					app.up_button = gtk_button_new();
					gtk_signal_connect(GTK_OBJECT(app.up_button), "button_press_event",
							GTK_SIGNAL_FUNC(updir_click_cb), view);
					tooltips = gtk_tooltips_new();
					gtk_tooltips_set_tip(GTK_TOOLTIPS(tooltips), app.up_button,
							"Left Click: Up Dir   Right Click: Home", NULL);
					gtk_box_pack_end(GTK_BOX(hbox), app.up_button, FALSE, FALSE, 0);
					gtk_widget_show(app.up_button);

					style = gtk_widget_get_style(app.main_window);
					pixmap = gdk_pixmap_create_from_xpm_d(app.main_window->window, &mask,
							&style->bg[GTK_STATE_NORMAL], icon_dirparent_xpm);
					pixmapwid = gtk_pixmap_new(pixmap, mask);
					gtk_container_add(GTK_CONTAINER(app.up_button), pixmapwid);
					gtk_widget_show(pixmapwid);
				}


			}
			else
			{
      gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);

				gtk_widget_show(label);
			}
			

      gtk_widget_show (hbox);
      gtk_clist_set_column_widget (GTK_CLIST (view->clist), i, hbox);
    }
  gtk_clist_column_titles_show (GTK_CLIST (view->clist));
  gtk_clist_set_shadow_type (GTK_CLIST (view->clist), GTK_SHADOW_ETCHED_IN);
  gtk_clist_set_selection_mode (GTK_CLIST (view->clist),
				GTK_SELECTION_EXTENDED);
  gtk_clist_set_use_drag_icons (GTK_CLIST (view->clist), TRUE);
  gtk_clist_set_row_height (GTK_CLIST (view->clist), 16);

  for (i = 0; i < MAX_COLUMNS; i++)
    {
      gtk_clist_set_column_width (GTK_CLIST (view->clist), i,
				  all_columns[i].size);
      gtk_clist_set_column_visibility (GTK_CLIST (view->clist), i,
				       all_columns[i].is_visible);
    }

  gtk_signal_connect (GTK_OBJECT (view->clist), "select_row",
		      GTK_SIGNAL_FUNC (select_row_cb), view);
	gtk_signal_connect(GTK_OBJECT(view->clist), "button_press_event",
			GTK_SIGNAL_FUNC(mouse_cb), view);

	gtk_signal_connect(GTK_OBJECT(view->clist), "click_column",
			GTK_SIGNAL_FUNC(column_button_cb), view);

	  key_press_signal = gtk_signal_connect (GTK_OBJECT (view->clist),
			"key-press-event", GTK_SIGNAL_FUNC (file_list_key_press_cb), view);
	gtk_object_set_data(GTK_OBJECT(view->clist), "signal_key_id",
			GUINT_TO_POINTER(key_press_signal));
	

  gtk_drag_dest_set (view->clist, GTK_DEST_DEFAULT_ALL, target_table,
		     n_targets, GDK_ACTION_ASK);

  gtk_container_add (GTK_CONTAINER (sw), view->clist);
  gtk_widget_show (view->clist);

  /* Set the CLIST_COLOR for resetting from the DRAG_HILIGHT color */
  {
    GtkStyle *style = gtk_widget_get_style (view->clist);
    CLIST_COLOR.red = style->base[GTK_STATE_NORMAL].red;
    CLIST_COLOR.green = style->base[GTK_STATE_NORMAL].green;
    CLIST_COLOR.blue = style->base[GTK_STATE_NORMAL].blue;
  }

  return vbox;
}

GtkWidget *
create_command_view (void)
{
  GtkWidget *sw;
	GtkWidget *frame;
	guint selection_signal;

	frame = gtk_frame_new(NULL);
	gtk_box_pack_start(GTK_BOX(app.hbox), frame, FALSE, FALSE, 0);
	gtk_widget_show(frame);
  sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add(GTK_CONTAINER(frame), sw);
	if(cfg.use_scrollbars)
	{
  	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	}
	else
	{
  	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_NEVER, GTK_POLICY_NEVER);
	}

  gtk_widget_show (sw);
  command_view = &app.command_view;
  command_view->clist = gtk_clist_new (1);
  gtk_clist_set_selection_mode (GTK_CLIST (command_view->clist),
				GTK_SELECTION_BROWSE);

  gtk_container_add (GTK_CONTAINER (sw), command_view->clist);
  gtk_clist_set_shadow_type (GTK_CLIST (command_view->clist),
			     GTK_SHADOW_NONE);
  gtk_clist_column_titles_show (GTK_CLIST (command_view->clist));
  app.label = gtk_label_new (" Commands ");
  gtk_widget_show (app.label);

  gtk_clist_set_column_widget (GTK_CLIST (command_view->clist), 0, app.label);
  gtk_widget_show (command_view->clist);
  load_user_commands ();
  selection_signal = gtk_signal_connect (GTK_OBJECT (command_view->clist),
			"select_row", GTK_SIGNAL_FUNC (command_select_row_cb), command_view);
	gtk_signal_connect(GTK_OBJECT(command_view->clist), "select_row",
			GTK_SIGNAL_FUNC(mouse_command_cb), command_view);
	gtk_object_set_data(GTK_OBJECT(command_view->clist), "signal_key_id",
			GUINT_TO_POINTER(selection_signal));
  gtk_signal_connect (GTK_OBJECT (command_view->clist), "key_press_event",
		      GTK_SIGNAL_FUNC (file_list_key_press_cb), command_view);
  return sw;

}

/* Main Window Setup */
void
create_main_window ()
{

  app.main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_signal_connect (GTK_OBJECT (app.main_window), "delete_event",
		      GTK_SIGNAL_FUNC (quit_cb), NULL);

  gtk_signal_connect (GTK_OBJECT (app.main_window),"key_press_event",
			GTK_SIGNAL_FUNC (window_key_press_cb), NULL);

  gtk_widget_set_usize (app.main_window, cfg.window_width, cfg.window_height);
	if(cfg.save_position)
  	gtk_widget_set_uposition (GTK_WIDGET (app.main_window), cfg.window_xpos,
			    cfg.window_ypos);

  gtk_window_set_policy (GTK_WINDOW (app.main_window), TRUE, TRUE, FALSE);
  gtk_widget_realize (app.main_window);

	app.notebook = gtk_notebook_new();
	gtk_container_add(GTK_CONTAINER(app.main_window), app.notebook);
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(app.notebook), GTK_POS_BOTTOM);

	app.notebook_label = gtk_label_new("1 Vide");
	gtk_widget_show(app.notebook_label);

  app.main_window_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show(app.notebook);
  gtk_widget_show (app.main_window_vbox);

	gtk_notebook_append_page(GTK_NOTEBOOK(app.notebook), app.main_window_vbox,
			app.notebook_label);
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(app.notebook), zvt.show_tabs);
	gtk_signal_connect(GTK_OBJECT(app.notebook), "switch-page",
			GTK_SIGNAL_FUNC(notebook_cb), NULL);



  /* menubar */
  app.main_menu_bar = create_main_menu_bar ();
  gtk_widget_show (app.main_menu_bar);
  gtk_box_pack_start (GTK_BOX (app.main_window_vbox), app.main_menu_bar,
		      FALSE, FALSE, 0);


  app.hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (app.hbox);

  /* Left Panel */
  app.left_view_box = create_file_view (&app.left_view);
  curr_view = &app.left_view;
  gtk_box_pack_start (GTK_BOX (app.hbox), app.left_view_box, TRUE, TRUE, 0);

  /* Command clist */
  app.sw = create_command_view ();

  /* Right Panel */
  app.right_view_box = create_file_view (&app.right_view);
  other_view = &app.right_view;
  gtk_box_pack_start (GTK_BOX (app.hbox), app.right_view_box, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (app.main_window_vbox), app.hbox, TRUE, TRUE,
		      0);

  app.sep = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (app.main_window_vbox), app.sep, FALSE, FALSE,
		      0);
  gtk_widget_show (app.sep);



  /* Staus Bar */
	app.message_box = gtk_hbox_new(TRUE, 0);
	gtk_widget_show(app.message_box);
	gtk_box_pack_start(GTK_BOX(app.main_window_vbox), app.message_box,
			FALSE, FALSE, 0);
	gtk_box_set_homogeneous(GTK_BOX(app.message_box), FALSE);
	
  app.status_bar = add_label (app.message_box, "Ready", 0.5, TRUE, 0);
  gtk_misc_set_alignment (GTK_MISC (app.status_bar), 0.02, 0.5);
  gtk_widget_show (GTK_WIDGET (app.status_bar));

	app.sep = gtk_hseparator_new();
	gtk_box_pack_end(GTK_BOX(app.main_window_vbox), app.sep, FALSE, FALSE, 0);
	gtk_widget_show(app.sep);


  load_bookmarks ();

  gtk_widget_show (app.main_window);

}

void
recreate_main_window ()
{
  FileView *last_view;
  GtkAdjustment *vadj;
  gfloat scrollbar_pos;
  gint last_selected_row;
	gint page;

  /* Save some info on the current view so we can reset it when done */
  last_view = curr_view;
  scrollbar_pos =
    gtk_clist_get_vadjustment (GTK_CLIST (curr_view->clist))->value;
  last_selected_row = curr_view->row;

  gtk_widget_destroy (app.main_menu_bar);
  gtk_widget_destroy (app.left_view_box);
  gtk_widget_destroy (app.right_view_box);
  gtk_widget_destroy (app.sw);
  gtk_widget_destroy (app.hbox);
  gtk_widget_destroy (app.message_box);
  gtk_widget_destroy (app.sep);

  app.main_menu_bar = create_main_menu_bar ();
  gtk_widget_show (app.main_menu_bar);
  gtk_box_pack_start (GTK_BOX (app.main_window_vbox), app.main_menu_bar,
		      FALSE, FALSE, 0);

  app.hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (app.main_window_vbox), app.hbox, TRUE, TRUE,
		      0);
  gtk_widget_show (app.hbox);

  /* Left Panel */
  app.left_view_box = create_file_view (&app.left_view);
  gtk_box_pack_start (GTK_BOX (app.hbox), app.left_view_box, TRUE, TRUE, 0);

  /* command Panel */
  app.sw = create_command_view ();


  /* Right Panel */
  app.right_view_box = create_file_view (&app.right_view);
  gtk_box_pack_start (GTK_BOX (app.hbox), app.right_view_box, TRUE, TRUE, 0);

  app.sep = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (app.main_window_vbox), app.sep, FALSE, FALSE,
		      0);
  gtk_widget_show (app.sep);

  /* Staus Bar */
	app.message_box = gtk_hbox_new(TRUE, 0);
	gtk_widget_show(app.message_box);
	gtk_box_pack_start(GTK_BOX(app.main_window_vbox), app.message_box,
			FALSE, FALSE, 0);
	
	gtk_box_set_homogeneous(GTK_BOX(app.message_box), FALSE);
  app.status_bar = add_label (app.message_box, "Ready", 0.5, TRUE, 0);
  gtk_misc_set_alignment (GTK_MISC (app.status_bar), 0.02, 0.5);
  gtk_widget_show (GTK_WIDGET (app.status_bar));

  load_bookmarks ();

  change_dir(&app.right_view, app.right_view.dir);
  change_dir(&app.left_view, app.left_view.dir);
  sort_list (&app.right_view, name_sort, GTK_SORT_ASCENDING, 0);
  sort_list (&app.left_view, name_sort, GTK_SORT_ASCENDING, 0);

  gtk_widget_show (app.hbox);

	gtk_widget_grab_focus(last_view->clist);
  gtk_clist_unselect_all (GTK_CLIST (last_view->clist));
  gtk_clist_select_row (GTK_CLIST (last_view->clist), last_selected_row, 0);
  GTK_CLIST (last_view->clist)->focus_row = last_selected_row;
	page = gtk_notebook_get_current_page(GTK_NOTEBOOK(app.notebook));
	if(page == 0)
		gtk_widget_draw_focus(last_view->clist);
	else
		gtk_widget_grab_focus(GTK_WIDGET(get_nth_zvt(
						GTK_NOTEBOOK(app.notebook), page)));
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(app.notebook), zvt.tab_position);
	update_all_terms();
		
  vadj = gtk_clist_get_vadjustment (GTK_CLIST (last_view->clist));
  gtk_adjustment_set_value (GTK_ADJUSTMENT (vadj), scrollbar_pos);
}
