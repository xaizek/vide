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


typedef int VTTITLE_TYPE;

static void
change_title(ZvtTerm *term, VTTITLE_TYPE type, char *newtitle,
		GtkWidget *term_window)
{
	gtk_window_set_title(GTK_WINDOW(term_window), newtitle);
}

static void
close_term_window(ZvtTerm *term, gpointer data)
{
	/* zvt_term_closepty(term); */
	gtk_widget_destroy(GTK_WIDGET(term)->parent);
}

static void
detach_term(GtkWidget *widget, ZvtTerm *term)
{
	GtkWidget *term_window;
	GtkWidget *menu_bar;
	GtkWidget *vbox;
	GtkWidget *term_label;
	gchar *label_text;
	gint term_count;
	guint child_died_signal_id;
	guint title_change_signal;

	vbox = gtk_object_get_data(GTK_OBJECT(term), "vbox");
	term_count = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(vbox),
				"term_count"));
	child_died_signal_id = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(term),
				"child_died_signal_id"));
	gtk_signal_disconnect(GTK_OBJECT(term), child_died_signal_id);
	title_change_signal = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(term),
				"title_change_signal"));
	gtk_signal_disconnect(GTK_OBJECT(term), title_change_signal);
	term_label = gtk_object_get_data(GTK_OBJECT(term), "term_label");
	gtk_label_get(GTK_LABEL(term_label), &label_text);

	term_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(term_window), label_text);
	gtk_signal_connect(GTK_OBJECT(term_window), "delete-event",
			GTK_SIGNAL_FUNC(gtk_widget_destroy), term_window);
	gtk_widget_realize(term_window);


	menu_bar = gtk_object_get_data(GTK_OBJECT(term), "menubar");
	gtk_widget_destroy(menu_bar);

	/* Need to use ref and unref with reparent here - don't know why? */
	gtk_widget_ref(GTK_WIDGET(term));
	gtk_widget_reparent(GTK_WIDGET(term), term_window);
	gtk_widget_unref(GTK_WIDGET(term));

	gtk_signal_connect(GTK_OBJECT(term), "child_died",
			GTK_SIGNAL_FUNC(close_term_window), NULL);
	gtk_signal_connect(GTK_OBJECT(term), "title_changed",
			GTK_SIGNAL_FUNC(change_title), term_window);

	
	gtk_widget_grab_focus(GTK_WIDGET(term));

	gtk_widget_show(term_window);

	if(term_count > 1)
	{
		GList *child, *tmp;
		ZvtTerm *tmp_term;
		GtkWidget *ch;
		GtkWidget *page_label;
		GtkWidget *term_label;
		char *label_text;
		gchar buf[NAME_MAX];

		term_count--;
		gtk_object_set_data(GTK_OBJECT(vbox), "term_count",
				GUINT_TO_POINTER(term_count));
		child = GTK_BOX(vbox)->children;

		term_count = 0;
		for(tmp = child; tmp != NULL; tmp = tmp->next)
		{
			ch = ((GtkBoxChild*)(tmp->data))->widget;
			/* if(ZVT_IS_TERM(ch)) */
			/* { */
			/* 	term_count++; */
			/* 	gtk_object_set_data(GTK_OBJECT(ch), "term_number", */
			/* 				GUINT_TO_POINTER(term_count)); */
			/* } */
		}
		tmp_term = get_nth_zvt(GTK_NOTEBOOK(app.notebook),
				gtk_notebook_get_current_page(GTK_NOTEBOOK(app.notebook)));
		term_count = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(tmp_term),
					"term_number"));
		gtk_object_set_data(GTK_OBJECT(vbox), "focus_term", 
				GUINT_TO_POINTER(term_count));
		cfg.current_term = tmp_term;
		term_label = gtk_object_get_data(GTK_OBJECT(tmp_term), "term_label");
	 	page_label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(app.notebook),
		gtk_notebook_get_nth_page(GTK_NOTEBOOK(app.notebook), 
		gtk_notebook_get_current_page(GTK_NOTEBOOK(app.notebook))));
		gtk_label_get(GTK_LABEL(term_label), &label_text);
		g_snprintf(buf, sizeof(buf), "%d %s",
				gtk_notebook_get_current_page(GTK_NOTEBOOK(app.notebook)) +1,
			 label_text);
		gtk_label_set_text(GTK_LABEL(page_label), buf);


		gtk_widget_grab_focus(GTK_WIDGET(tmp_term));


	}
	else
	{
		gint page;
		gtk_widget_destroy(GTK_WIDGET(vbox));
		gtk_notebook_set_page(GTK_NOTEBOOK(app.notebook), -1);
		page = gtk_notebook_get_current_page(GTK_NOTEBOOK(app.notebook));
		if(page == 0)
		{
			cfg.term_count = 0;
			cfg.current_term = NULL;
		}
	}
}

static void
split_zterm(GtkWidget *widget, gpointer data)
{
	create_zterm("sh", FALSE);
}

static void
menu_position(GtkMenu *menu, gint *x, gint *y, gpointer data)
{
	 gdk_window_get_origin((app.main_window)->window, x, y);
   *x += 4;
   *y += 30;
}

static void
lookup_word(GtkWidget *widget, ZvtTerm* term)
{
	gchar buf[256];
	/* gchar * word = zvt_term_get_buffer(term, NULL, VT_SELTYPE_WORD, */
	/* 		term->vx->vt.cursorx, term->vx->vt.cursory, */
	/* 		term->vx->vt.cursorx, term->vx->vt.cursory); */
	/* g_snprintf(buf, sizeof(buf), "%s %s", cfg.man_program, word); */
	/* create_zterm(buf, FALSE); */
	/* g_free(word); */
}

/*
static void
compare_file(GtkWidget *widget, ZvtTerm *term)
{
	* get file name from fileselector
	 * get current filename from term
	 * call vimdiff in new term 
	 * vsplit calling vim with new file
}
*/

/*
static void
copy_text(GtkWidget *widget, ZvtTerm *term)
{
	* use !vide_copy to filter text to vide clipboard_file 
	 * return text as originally passed to program 
}
*/

/*
static void
paste_text(GtkWidget *widget, ZvtTerm *term)
{
	 read vide clipboard_file and write to term to paste text 
}
*/
	
static void
create_page_menu(GtkWidget *widget, gpointer data)
{
	create_buffer_menu();
	gtk_menu_popup(GTK_MENU(app.buffer_menu), NULL, NULL,
			         menu_position, NULL, 0, 0);
}
	 

static void
menu_key_cb(GtkWidget *widget, GdkEventKey *event, FileView *view)
{
		/* a - Z */
	if((event->keyval < 0x100) && (event->state == 0 || event->state & GDK_SHIFT_MASK 
				|| event->state & GDK_MOD2_MASK))
	{
		switch(event->keyval)
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
				break;
		}
	}
}

GtkWidget *
create_term_menu(ZvtTerm *term, gchar *command)
{
	GtkWidget *menu_bar;
	GtkWidget *menu;
	GtkWidget *menu_item;
	GtkWidget *vide_menu;
	GtkWidget *tools_menu;
	GtkWidget *term_label;
	GdkColor color = TAB_COLOR;
	GtkStyle *defstyle;
	GtkStyle *style = gtk_style_new();

	menu_bar = gtk_menu_bar_new();

	defstyle = gtk_widget_get_default_style();
	style = gtk_style_copy(defstyle);
	style->fg[0] = color;

	menu = gtk_menu_new();
	gtk_signal_connect(GTK_OBJECT(menu), "key_press_event",
			GTK_SIGNAL_FUNC(menu_key_cb), NULL);

	gtk_object_set_data(GTK_OBJECT(term), "menu_bar", menu_bar);

	
	/* The terminal is running vi */
	if(GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(term))))
	{
		add_menu_item(menu, "Open File in Buffer", create_fileselector,
				GUINT_TO_POINTER(0));
		if(strcmp(cfg.vi_clone, "vi"))
  		add_menu_item (menu, "Split Buffer & Open File", create_fileselector,
					GUINT_TO_POINTER(1));
		if(!strcmp(cfg.vi_clone, "vim"))
			add_menu_item (menu, "VSplit Buffer & Open File", create_fileselector,
					GUINT_TO_POINTER(2));
  	add_menu_separator (menu);
	}
	add_menu_item(menu, "Open File in New Term", 
			create_fileselector, GUINT_TO_POINTER(3));
  add_menu_separator (menu);
	add_menu_item(menu, "New Shell", split_zterm, NULL);
	add_menu_separator(menu);
	add_menu_item(menu, "List Pages", create_page_menu, NULL);
	add_menu_separator(menu);
	add_menu_item(menu, "Detach Term", detach_term, term);
	add_menu_separator(menu);
  add_menu_item (menu, "Cancel", NULL, NULL);
  vide_menu = add_submenu (menu_bar, "_Vide", menu);
	gtk_object_set_data(GTK_OBJECT(term), "vide_menu", vide_menu);

	menu = gtk_menu_new();
	gtk_signal_connect(GTK_OBJECT(menu), "key_press_event",
			GTK_SIGNAL_FUNC(menu_key_cb), NULL);
	add_menu_item(menu, "Look up word", lookup_word, term);
	add_menu_separator(menu);

	/* 
	add_menu_item(menu, "Compare file to..", compare_file, term);
	add_menu_separator(menu);
	*/

	/* I don't know how to visually select text in nvi and vile 
	if((!strcmp("vim", cfg.vi_clone) | (!strcmp("elvis", cfg.vi_clone))))
	{
		add_menu_item(menu, "Copy", copy_text, term);
		add_menu_separator(menu);
		add_menu_item(menu, "Paste", paste_text, term);
		add_menu_separator(menu);
	}
	*/
	add_menu_item(menu, "Cancel", NULL, NULL);
	tools_menu = add_submenu(menu_bar, "_Tools", menu);
	gtk_object_set_data(GTK_OBJECT(term), "tools_menu", tools_menu);

	/* label for filename on menubar */
	menu = gtk_menu_new();
	menu_item = gtk_menu_item_new();
	term_label = gtk_widget_new(GTK_TYPE_LABEL,
			"GtkWidget::visible", TRUE,
			"GtkWidget::parent", menu_item,
			"GtkMisc::xalign", 0, 0, NULL);
	gtk_label_set_text(GTK_LABEL(term_label), command);
	gtk_widget_set_style(GTK_WIDGET(term_label), style);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
	gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), menu_item);
	gtk_menu_item_right_justify(GTK_MENU_ITEM(menu_item));
	gtk_widget_show(menu_item);
	gtk_object_set_data(GTK_OBJECT(term), "term_label", term_label);


	return  menu_bar;


}


