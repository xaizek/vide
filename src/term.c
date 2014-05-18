/* vide																			
 * Copyright (C) 2000 Ken Steen.				vim600:fdm=marker
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
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <stdlib.h>
#include <signal.h>
#include "vide.h"

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkprivate.h>
#include <gdk/gdkkeysyms.h>
#include<gnome.h>
#include <zvt/vt.h>
#include<X11/Xatom.h>


static void
term_key_cb(GtkWidget *widget, GdkEventKey *event, ZvtTerm *term);

void
write_to_term(ZvtTerm *term, gchar *message)
{
	write(term->vx->vt.keyfd, message, strlen(message));
}

void
write_to_all_vi_terms(gchar *message)
{
	ZvtTerm *term;
	gchar buf[NAME_MAX];
	gint page = cfg.term_count;

	/* Write to the last page in the notebook.  If it is a 
	 * quit command the page numbers will change otherwise
	 */

	if(!page)
		return;

	while((term = get_nth_zvt(GTK_NOTEBOOK(app.notebook), page)))
	{
		if(GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(term))))
		{
			g_snprintf(buf, sizeof(buf), "%c\r", GDK_Escape);
			write_to_term(term, buf);
			write_to_term(term, message);
		}
		page--;
		if(page < 1)
			break;
	}
}


/* Common terminal colors from the Gnome-terminal {{{ */
gushort linux_red[] = { 0x0000, 0xaaaa, 0x0000, 0xaaaa, 0x0000, 0xaaaa, 0x0000, 0xaaaa,
			0x5555, 0xffff, 0x5555, 0xffff, 0x5555, 0xffff, 0x5555, 0xffff,
			0x0,    0x0 };
gushort linux_grn[] = { 0x0000, 0x0000, 0xaaaa, 0x5555, 0x0000, 0x0000, 0xaaaa, 0xaaaa,
			0x5555, 0x5555, 0xffff, 0xffff, 0x5555, 0x5555, 0xffff, 0xffff,
			0x0,    0x0 };
gushort linux_blu[] = { 0x0000, 0x0000, 0x0000, 0x0000, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
			0x5555, 0x5555, 0x5555, 0x5555, 0xffff, 0xffff, 0xffff, 0xffff,
			0x0,    0x0 };

gushort xterm_red[] = { 0x0000, 0x6767, 0x0000, 0x6767, 0x0000, 0x6767, 0x0000, 0x6868,
			0x2a2a, 0xffff, 0x0000, 0xffff, 0x0000, 0xffff, 0x0000, 0xffff,
			0x0,    0x0 };

gushort xterm_grn[] = { 0x0000, 0x0000, 0x6767, 0x6767, 0x0000, 0x0000, 0x6767, 0x6868,
			0x2a2a, 0x0000, 0xffff, 0xffff, 0x0000, 0x0000, 0xffff, 0xffff,
			0x0,    0x0 };
gushort xterm_blu[] = { 0x0000, 0x0000, 0x0000, 0x0000, 0x6767, 0x6767, 0x6767, 0x6868,
			0x2a2a, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff,
			0x0,    0x0 };

gushort rxvt_red[] = { 0x0000, 0xffff, 0x0000, 0xffff, 0x0000, 0xffff, 0x0000, 0xffff,
		       0x0000, 0xffff, 0x0000, 0xffff, 0x0000, 0xffff, 0x0000, 0xffff,
			0x0,    0x0 };
gushort rxvt_grn[] = { 0x0000, 0x0000, 0xffff, 0xffff, 0x0000, 0x0000, 0xffff, 0xffff,
		       0x0000, 0x0000, 0xffff, 0xffff, 0x0000, 0x0000, 0xffff, 0xffff,
			0x0,    0x0 };
gushort rxvt_blu[] = { 0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff,
		       0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff,
			0x0,    0x0 };

gushort *scheme_red[] = { linux_red, xterm_red, rxvt_red, rxvt_red };
gushort *scheme_blu[] = { linux_blu, xterm_blu, rxvt_blu, rxvt_blu };
gushort *scheme_grn[] = { linux_grn, xterm_grn, rxvt_grn, rxvt_grn };


static void
set_color_scheme(ZvtTerm *term)
{
	GdkColor c;
	gushort red[18], green[18], blue[18];
	int i;
	gushort *r = NULL;
	gushort *b = NULL;
	gushort *g = NULL;
	gint scheme = 0;
	 
	if(!strcmp(zvt.term_color, "Linux"))
	{
		r = scheme_red[0];
		g = scheme_grn[0];
		b = scheme_blu[0];
		scheme = 1;
	}
	if(!strcmp(zvt.term_color, "Xterm"))
	{
		r = scheme_red[1];
		g = scheme_grn[1];
		b = scheme_blu[1];
		scheme = 1;
	}
	if(!strcmp(zvt.term_color, "Rxvt"))
	{
		r = scheme_red[2];
		g = scheme_grn[2];
		b = scheme_blu[2];
		scheme = 1;
	}
	if(!strcmp(zvt.term_color, "Custom"))
	{
		for(i = 0; i < 18; i++)
		{
			red[i] = zvt.palette[i].red;
			green[i] = zvt.palette[i].green;
			blue[i] = zvt.palette[i].blue;
		}
		scheme = 0;
	}

	if(scheme)
	{
		for(i = 0; i < 18; i++)
		{
			red[i] = r[i];
			green[i] = g[i];
			blue[i] = b[i];
		}
	}
	if(!strcmp(zvt.background, "WhiteOnBlack"))
	{
		red[16] = red[7];
		blue[16] = blue[7];
		green[16] = green[7];
		red[17] = red[0];
		blue[17] = blue[0];
		green[17] = green[0];
	}
	if(!strcmp(zvt.background, "BlackOnWhite"))
	{
		red[16] = red[0];
		blue[16] = blue[0];
		green[16] = green[0];
		red[17] = red[7];
		blue[17] = blue[7];
		green[17] = green[7];
	}
	if(!strcmp(zvt.background, "GreenOnBlack"))
	{
		red[17] = 0;
		blue[17] = 0;
		green[17] = 0;
		red[16] = 0;
		blue[16] = 0;
		green[16] = 0xffff;
	}
	if(!strcmp(zvt.background, "CustomBack"))
	{ 
		for(i = 16; i < 18; i++)
		{
			red[i] = zvt.palette[i].red;
			green[i] = zvt.palette[i].green;
			blue[i] = zvt.palette[i].blue;
		}
	}


	zvt_term_set_color_scheme((ZvtTerm *)term, red, green, blue);
	c.pixel = term->colors[17];
	gdk_window_set_background(GTK_WIDGET(term)->window, &c);
	gtk_widget_queue_draw(GTK_WIDGET(term));
}

/* }}}  just a marker for vim6.0 folding text*/

ZvtTerm *
get_focus_zvt(GtkNotebook *notebook, gint page_number)
{
	GList *child, *tmp;
	GtkWidget *ch;
	GtkWidget *vbox = NULL;
	gint focus_term;

	vbox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), page_number);
	if(!vbox)
		return NULL;

	child = GTK_BOX(vbox)->children;
	focus_term = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(vbox), 
				"focus_term"));

	for(tmp = child; tmp != NULL; tmp = tmp->next)
	{
		ch = ((GtkBoxChild*)(tmp->data))->widget;

		if(ZVT_IS_TERM(ch))
		{
			gint term_number = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(ch),
						"term_number"));
			if(term_number == focus_term)
			{
				return ZVT_TERM(ch);
			}
		}
	}
	return NULL;


}

ZvtTerm *
get_nth_zvt(GtkNotebook *notebook, gint n)
{
	GList *child, *tmp;
	GtkWidget *ch;
	GtkWidget *vbox = NULL;

	vbox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), n);
	if(!vbox)
		return NULL;

	child = GTK_BOX(vbox)->children;

	for(tmp = child; tmp != NULL; tmp = tmp->next)
	{
		ch = ((GtkBoxChild*)(tmp->data))->widget;

		if(ZVT_IS_TERM(ch))
			return ZVT_TERM(ch);
	}
	return NULL;
}

void UpdateTerm (ZvtTerm * term)
{
   if(zvt.use_pixmap)
	 {
		 zvt_term_set_background(ZVT_TERM(term), zvt.pixmap,
				 zvt.transparent, ZVT_BACKGROUND_SHADED);
	 }
	 else if(zvt.transparent)
	 {
			zvt_term_set_background(term, NULL, zvt.transparent, 0);
	 }
	 else
		 zvt_term_set_background(ZVT_TERM(term), NULL, 0, 0);

  zvt_term_set_font_name(ZVT_TERM (term), zvt.term_font);
  zvt_term_set_scrollback(ZVT_TERM (term), zvt.scroll_lines);
  zvt_term_set_blink (ZVT_TERM (term), zvt.use_blinking_cursor);
  zvt_term_set_bell (ZVT_TERM (term), zvt.terminal_bell);

  set_color_scheme (ZVT_TERM(term));


  zvt_term_set_scroll_on_keystroke (ZVT_TERM (term), zvt.scroll_on_keystroke);
  zvt_term_set_scroll_on_output (ZVT_TERM (term), zvt.scroll_on_output);

  gtk_widget_queue_draw (GTK_WIDGET(term));
}

void update_all_terms()
{
  ZvtTerm * ch;
  int a=1;
  
  while ((ch = get_nth_zvt(GTK_NOTEBOOK(app.notebook),a)))
    {
      UpdateTerm (ZVT_TERM(ch));
      a++;
    }
  
}

 static void
 child_died_event (ZvtTerm *term, GtkWidget *vbox)
 {
	 gint page;
	 ZvtTerm *tmp_term;
	 GtkWidget *label;
	 gint term_count;


	 term_count = (GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(vbox), 
				 "term_count")) -1);
	 if(term_count)
	 {
		 GList *child, *tmp;
		 ZvtTerm *tmp_term;
		 GtkWidget *ch;
		 GtkWidget *page_label;
		 GtkWidget *term_label;
		 gchar *label_text;
		 gchar buf[NAME_MAX];
		 gint term_number = 0;
		 GtkWidget *menubar = gtk_object_get_data(GTK_OBJECT(term), "menubar");

		 gtk_widget_destroy(menubar);

		 gtk_widget_hide(GTK_WIDGET(term));

		 zvt_term_closepty(term);
		 gtk_widget_destroy(GTK_WIDGET(term));
		 gtk_object_set_data(GTK_OBJECT(vbox), "term_count",
				 GUINT_TO_POINTER(term_count));
		 page = gtk_notebook_get_current_page(GTK_NOTEBOOK(app.notebook));
		 child = GTK_BOX(vbox)->children;

		 for(tmp = child; tmp != NULL; tmp = tmp->next)
		 {
			 ch = ((GtkBoxChild*)(tmp->data))->widget;
			 if(ZVT_IS_TERM(ch))
			 {
				 term_number++;
				 gtk_object_set_data(GTK_OBJECT(ch), "term_number",
						 GUINT_TO_POINTER(term_number));
			 }
		 }
		 /* set focus_term and notebook tab */

		 tmp_term = get_nth_zvt(GTK_NOTEBOOK(app.notebook), page);
		 term_number = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(tmp_term),
					 "term_number"));
		 gtk_object_set_data(GTK_OBJECT(vbox), "focus_term",
				 GUINT_TO_POINTER(term_number));
		 cfg.current_term = tmp_term;
		 term_label = gtk_object_get_data(GTK_OBJECT(tmp_term),
				 "term_label");
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
		 gtk_widget_hide(GTK_WIDGET(term));
		 gtk_container_remove(GTK_CONTAINER(app.notebook),
				 GTK_WIDGET(term)->parent);
		 gtk_notebook_set_page(GTK_NOTEBOOK(app.notebook), -1);
		 page = gtk_notebook_get_current_page(GTK_NOTEBOOK(app.notebook));

		if(page == 0)
	 	{
				cfg.term_count = 0;
				gtk_notebook_set_show_tabs(GTK_NOTEBOOK(app.notebook), zvt.show_tabs);
				cfg.current_term = NULL;
		 }
		 else
		 {
			 if(cfg.current_term == term)
				 cfg.current_term = NULL;
			 cfg.term_count--;
		 }

		 zvt_term_closepty(term);
		 /* reset the notebook tabs and vi buffer list */
		 page = 1;
		 while((tmp_term = get_nth_zvt(GTK_NOTEBOOK(app.notebook), page)))
		 {
			 gchar *title;
			 gchar *text;
			 gchar buf[128];

			 if(GPOINTER_TO_INT(gtk_object_get_user_data(GTK_OBJECT(tmp_term))))
			 {
		 		label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(app.notebook),
					 gtk_notebook_get_nth_page(GTK_NOTEBOOK(app.notebook), page));
		 		gtk_label_get(GTK_LABEL(label), &text);
		 		title = strstr(text, " ") + 1;
		 		if(page + 1 == 10)
		 			g_snprintf(buf, sizeof(buf), "%d %s", 0, title);
		 		else
		 			g_snprintf(buf, sizeof(buf), "%d %s", page + 1, title);
		 		gtk_label_set_text(GTK_LABEL(label), buf);
				if(cfg.current_term == NULL)
					cfg.current_term = tmp_term;
			 }
			 else
			 {
				 label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(app.notebook),
						 gtk_notebook_get_nth_page(GTK_NOTEBOOK(app.notebook), page));
				 gtk_label_get(GTK_LABEL(label), &text);
				 title = strstr(text, " ") +1;
				 if(page + 1 == 10)
					 g_snprintf(buf, sizeof(buf), "%d %s", 0, title);
				 else
				 {
					 g_snprintf(buf, sizeof(buf), "%d %s", page +1, title);
				 }
				 gtk_label_set_text(GTK_LABEL(label), buf);
		 	}
		 	page++;
	 	}
	 
	 		gtk_notebook_set_page(GTK_NOTEBOOK(app.notebook), 0);
	 		gtk_widget_grab_focus(GTK_WIDGET(curr_view->clist));
	 }
 }

static void
switch_term_key_cb(GtkWidget *widget, GdkEventKey *event, ZvtTerm *term)
{
	GtkWidget *vbox;
	guint key_press_signal;
	gint term_count;
	gint curr_number;
	gint next_term = -1;

	vbox = gtk_object_get_data(GTK_OBJECT(term), "vbox");
	term_count = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(vbox), "term_count"));
	if(term_count > 1)
	{
		curr_number = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(term),
					"term_number"));
		switch(event->keyval)
		{
			case 'j':
				next_term = curr_number -1;
				break;
			case 'k':
				next_term = curr_number +1;
				break;
			default:
				break;
		}
		if((next_term > 0) & (next_term <= term_count))
		{
			GList *child, *tmp;
			GtkWidget *ch;
			gint tmp_num = -2;

		 child = GTK_BOX(vbox)->children;
	 	 for(tmp = child; tmp != NULL; tmp = tmp->next)
		 {
			 ch = ((GtkBoxChild*)(tmp->data))->widget;
			 if(ZVT_IS_TERM(ch))
				 tmp_num = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(ch),
							 "term_number"));
			 if(tmp_num == next_term)
			 {
				 GtkWidget *term_label;
				 GtkWidget *page_label;
				 gchar buf[NAME_MAX];
				 gchar *label_text;

				 gint term_number = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(ch),
							 "term_number"));
				 term_label = gtk_object_get_data(GTK_OBJECT(ch),
						 "term_label");
				 page_label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(app.notebook),
						 gtk_notebook_get_nth_page(GTK_NOTEBOOK(app.notebook), 
							 gtk_notebook_get_current_page(GTK_NOTEBOOK(app.notebook))));
				 gtk_label_get(GTK_LABEL(term_label), &label_text);
				 g_snprintf(buf, sizeof(buf), "%d %s",
						 gtk_notebook_get_current_page(GTK_NOTEBOOK(app.notebook)) + 1,
						 label_text);
				 gtk_label_set_text(GTK_LABEL(page_label), buf);
				 gtk_object_set_data(GTK_OBJECT(vbox), "focus_term",
						 GUINT_TO_POINTER(term_number));
				 cfg.current_term = (ZvtTerm *)ch;
				 gtk_widget_grab_focus(GTK_WIDGET(ch));
				 break;
			 }
		 }
		}
	}
	switch(event->keyval)
	{
		case 'n':
			create_zterm("sh", FALSE);
			break;
		case 'm':
			{
				gchar buf[256];
				gchar * word = zvt_term_get_buffer(term, NULL, VT_SELTYPE_WORD,
						term->vx->vt.cursorx, term->vx->vt.cursory,
						term->vx->vt.cursorx, term->vx->vt.cursory);
				g_snprintf(buf, sizeof(buf), "%s %s", cfg.man_program, word);
				create_zterm(buf, FALSE);
				/*
				g_print("cursorx is %d\n", term->vx->vt.cursorx);
				g_print("cursory is %d\n", term->vx->vt.cursory);
				g_print("word under cursor is %s\n", word);
				*/
				g_free(word);
			}
			break;
		default:
			break;
	}


	gtk_signal_emit_stop_by_name(GTK_OBJECT(term), "key_press_event");
	key_press_signal = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(term),
				"signal_key_id"));
	gtk_signal_disconnect(GTK_OBJECT(term), key_press_signal);
	key_press_signal = gtk_signal_connect(GTK_OBJECT(term), "key_press_event",
			GTK_SIGNAL_FUNC(term_key_cb), term);
	gtk_object_set_data(GTK_OBJECT(term), "signal_key_id", 
			GUINT_TO_POINTER(key_press_signal));


}

static void
switch_terms(ZvtTerm *term)
{
	guint key_press_signal;

	key_press_signal = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(term),
				"signal_key_id"));
	gtk_signal_disconnect(GTK_OBJECT(term), key_press_signal);
	key_press_signal = gtk_signal_connect(GTK_OBJECT(term), "key_press_event",
			GTK_SIGNAL_FUNC(switch_term_key_cb), term);
	gtk_object_set_data(GTK_OBJECT(term), "signal_key_id",
			GUINT_TO_POINTER(key_press_signal));
}

static void
term_key_cb(GtkWidget *widget, GdkEventKey *event, ZvtTerm *term)
{

	if(event->state & GDK_CONTROL_MASK)
	{
		if((event->keyval >= '0') & (event->keyval <= '9'))
		{
			ZvtTerm *new_term;
			int page = event->keyval;


			if(page == '0')
			{
				page = 9;
				if((new_term = get_nth_zvt(GTK_NOTEBOOK(app.notebook), page)))
					cfg.current_term = new_term;
			}
			else
			{
				page = page - 49;
			}

			gtk_signal_emit_stop_by_name(GTK_OBJECT(term), "key_press_event");
			gtk_notebook_set_page(GTK_NOTEBOOK(app.notebook), page);

			if(page != 0)
			{
				new_term = get_focus_zvt(GTK_NOTEBOOK(app.notebook), page);
				cfg.current_term = new_term;
				gtk_widget_grab_focus(GTK_WIDGET(new_term));
			}
		}

	}
	if(event->state & GDK_MOD1_MASK)
	{
		GtkWidget *tools_menu = gtk_object_get_data(GTK_OBJECT(term), "tools_menu");
		GtkWidget *vide_menu = gtk_object_get_data(GTK_OBJECT(term), "vide_menu");


		if(vide_menu == NULL)
			return;

		switch(event->keyval)
		{
			case 't':
				gtk_signal_emit_by_name(GTK_OBJECT(tools_menu), "activate-item");
				break;
			case 'v':
			case 'V':
				gtk_signal_emit_by_name(GTK_OBJECT(vide_menu), "activate-item");
				break;
			case 'w':
			case 'W':
				switch_terms(term);
				break;
			default:
				break;
		}
		gtk_signal_emit_stop_by_name(GTK_OBJECT(term), "key_press_event");
	}
}


 static void
 title_changed_event (ZvtTerm *term, VTTITLE_TYPE type, char *newtitle)
 {
	 GtkWidget *label;
	 GtkWidget *term_label;
	 gint page;
	 gchar buf[512];


	 term_label = gtk_object_get_data(GTK_OBJECT(term), "term_label");
	 page = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(term),
				 "page_number"));
	 gtk_label_set_text(GTK_LABEL(term_label), newtitle);

	 label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(app.notebook),
			 gtk_notebook_get_nth_page(GTK_NOTEBOOK(app.notebook), page));
	 	page++;
		if(page == 10)
			page = 0;

	 	g_snprintf(buf, sizeof(buf), "%d %s", page, newtitle);
	 	gtk_label_set(GTK_LABEL(label), buf);
	 
 }




gint
create_zterm (gchar *command, gboolean new_page)
{
   gint  i, cmdindex, login_shell;
   char  **p;
   struct passwd *pw;
	 gchar buffer[40];
	 GtkWidget *vbox = NULL;
	 GtkWidget *menubar;
	 ZvtTerm *term;
	 gchar buf[NAME_MAX];
	 gchar term_label[NAME_MAX];
	 gint page = 1;
	 guint key_press_signal;
	 guint child_died_signal_id;
	 guint title_change_signal;

 	 extern char      **environ;
 	 static char      **env;
 	 static char      **env_copy;
 	 static int         winid_pos;


   login_shell = 0;
   cmdindex = 0;

	 signal(SIGHUP, SIG_IGN);



   
   /* Create widgets and set options */
	 page = cfg.term_count +2;
	 if(page == 10)
		 page = 0;

	 if(!strncmp(command, cfg.vi_command, strlen(cfg.vi_command)))
	 {
		 gchar *command_copy = command;
		 command_copy+=strlen(cfg.vi_command);
		 g_snprintf(buf, sizeof(buf), "%d %s", page, command_copy);
		 g_snprintf(term_label, sizeof(term_label), "%s", command_copy);
	 }
	 else
	 {
	 		g_snprintf(buf, sizeof(buf), "%d %s", page, command);
			g_snprintf(term_label, sizeof(term_label), "%s", command);
	 }
	 /* reset the page to the real notebook page number */
	 page = cfg.term_count +1;

   term = (ZvtTerm *)zvt_term_new_with_size(80,24);
	 gtk_object_set_data(GTK_OBJECT(term), "label_text", buf);

	 if(new_page)
	 {
   		app.notebook_label = gtk_label_new(buf);
	 		gtk_widget_show(app.notebook_label);
   		vbox = gtk_vbox_new (FALSE, 0);
   		gtk_box_set_spacing (GTK_BOX (vbox), 0);
   		gtk_container_set_border_width (GTK_CONTAINER (vbox), 0);
   		gtk_notebook_append_page(GTK_NOTEBOOK(app.notebook), vbox,
		 			app.notebook_label);
   		gtk_widget_show (vbox);
			gtk_object_set_data(GTK_OBJECT(vbox), "term_count", GUINT_TO_POINTER(1));
			gtk_object_set_data(GTK_OBJECT(term), "term_number", GUINT_TO_POINTER(1));
			gtk_object_set_data(GTK_OBJECT(term), "vbox", vbox);
			gtk_object_set_data(GTK_OBJECT(vbox), "focus_term", GUINT_TO_POINTER(1));
			gtk_object_set_data(GTK_OBJECT(term), "page_number", 
					GUINT_TO_POINTER(page));

	 }
	 else
	 {
		 gint term_count;
			vbox = gtk_notebook_get_nth_page(GTK_NOTEBOOK(app.notebook), 
					gtk_notebook_get_current_page(GTK_NOTEBOOK(app.notebook)));
			if(vbox == NULL)
				return 0;
			term_count = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(vbox),
						"term_count"));
			term_count++;
			gtk_object_set_data(GTK_OBJECT(vbox), "term_count",
					GUINT_TO_POINTER(term_count));
			gtk_object_set_data(GTK_OBJECT(term), "term_number",
					GUINT_TO_POINTER(term_count));
			gtk_object_set_data(GTK_OBJECT(term), "page_number", 
					GUINT_TO_POINTER(gtk_notebook_get_current_page(
							GTK_NOTEBOOK(app.notebook))));
			gtk_object_set_data(GTK_OBJECT(term), "vbox", vbox);
			gtk_object_set_data(GTK_OBJECT(vbox), "focus_term", 
					GUINT_TO_POINTER(term_count));
	 }


 

   /* set terminal options */
   gtk_widget_show (GTK_WIDGET(term));
   zvt_term_set_font_name(ZVT_TERM (term), zvt.term_font);
   zvt_term_set_blink (ZVT_TERM (term), zvt.use_blinking_cursor);
   zvt_term_set_bell (ZVT_TERM (term), zvt.terminal_bell);
   zvt_term_set_scrollback(ZVT_TERM (term), zvt.scroll_lines);
   zvt_term_set_scroll_on_keystroke (ZVT_TERM (term), zvt.scroll_on_keystroke);
   zvt_term_set_scroll_on_output (ZVT_TERM (term), zvt.scroll_on_output);
   zvt_term_set_wordclass (ZVT_TERM (term), "-A-Za-z0-9/_:.,?+%=");
	 if(zvt.use_pixmap)
	 {
		 zvt_term_set_background(ZVT_TERM(term), zvt.pixmap,
				 zvt.transparent, ZVT_BACKGROUND_SHADED);
	 }
	 else if(zvt.transparent)
	 {
			zvt_term_set_background(term, NULL, zvt.transparent, 0);
	 }
	 else
		 zvt_term_set_background(ZVT_TERM(term), NULL, 0, 0);

	 zvt_term_set_del_key_swap(ZVT_TERM(term), zvt.swap_backspace);
	 if(new_page)
	 	cfg.term_count++;
	 gtk_notebook_set_show_tabs(GTK_NOTEBOOK(app.notebook), zvt.show_tabs);

	 /* Check if this term is running vi
		* This is used to determine if you can use :b, :sp, or :vs 
		* commands to load files into this terminal
		*/
	 if(!strncmp(command, cfg.vi_command, strlen(cfg.vi_command)))
	 {
	 		gtk_object_set_user_data(GTK_OBJECT(term), GUINT_TO_POINTER(TRUE));
	 		cfg.current_term = (ZvtTerm *)term;
	 }
	 else
	 	gtk_object_set_user_data(GTK_OBJECT(term), GUINT_TO_POINTER(FALSE));



	 menubar = create_term_menu(term, term_label);
	 gtk_widget_show(menubar);
	 gtk_object_set_data(GTK_OBJECT(term), "menubar", menubar);

   gtk_box_pack_end (GTK_BOX (vbox), GTK_WIDGET(term), 1, 1, 0);

   gtk_box_pack_end (GTK_BOX (vbox), menubar, FALSE, FALSE, 0);

	 set_color_scheme((ZvtTerm*)term);

	 key_press_signal = gtk_signal_connect(GTK_OBJECT(term), "key_press_event",
			 GTK_SIGNAL_FUNC(term_key_cb), term);
	 gtk_signal_connect(GTK_OBJECT(term), "button_press_event",
			 GTK_SIGNAL_FUNC(gtk_widget_grab_focus), term);
	 gtk_object_set_data(GTK_OBJECT(term), "signal_key_id",
			 GUINT_TO_POINTER(key_press_signal));

   child_died_signal_id = gtk_signal_connect (GTK_OBJECT (term),
                       "child_died",
                       GTK_SIGNAL_FUNC (child_died_event),
                       vbox);
	 gtk_object_set_data(GTK_OBJECT(term), "child_died_signal_id", 
			 GUINT_TO_POINTER(child_died_signal_id));

   title_change_signal = gtk_signal_connect (GTK_OBJECT (term),
                       "title_changed",
                       GTK_SIGNAL_FUNC (title_changed_event),
                       NULL);
	 gtk_object_set_data(GTK_OBJECT(term), "title_change_signal",
			 GUINT_TO_POINTER(title_change_signal));


	 XSync(GDK_DISPLAY(), False);
 /* set up terminal environment */ 
   env = environ;


   for (p = env; *p; p++);
     i = p - env;
   env_copy = (char **) g_malloc (sizeof (char *) * (i + 3));
   for (i = 0, p = env; *p; p++) {
     if (strncmp (*p, "TERM=", 5) == 0) {
       env_copy [i++] = "TERM=xterm";
     } else if ((strncmp (*p, "COLUMNS=", 8) == 0) ||
                (strncmp (*p, "LINES=", 6) == 0)) {
       continue;
     } else {
       env_copy [i++] = *p;
     }
   }

   env_copy [i++] = "COLORTERM=color-xterm";
   winid_pos = i++;
   env_copy [winid_pos] = "TEST";
   env_copy [i] = NULL;


   /* fork the shell or program */
   switch (zvt_term_forkpty(ZVT_TERM (term), ZVT_TERM_DO_UTMP_LOG |  ZVT_TERM_DO_WTMP_LOG)) {
     case -1:
     perror("ERROR: unable to fork:");
     exit(1);
     break;

     case 0:
     { 
			 GString *shell, *name;
			 gchar *args[4];

       /* get shell from passwd */
       pw = getpwuid(getuid());
       if (pw) {
         shell = g_string_new(pw->pw_shell);
         if (login_shell)
           name = g_string_new("-");
         else
           name = g_string_new("");

         g_string_append(name, strrchr(pw->pw_shell, '/'));
       } else {
         shell = g_string_new("/bin/sh");
         if (login_shell)
           name = g_string_new("-sh");
         else
           name = g_string_new("sh");
       }

			 /*
			  * Without this the title change may not work.
				* I'm not sure why but if you launch vide from a shell this is 
				* not needed.  But if you launch it from a window manager menu
				* the title_changed event is not caught unless you specify the
				* term window id.  This is from the gnome-terminal application
				* and is undocumented in the zvt docs.
				*/
			 sprintf(buffer, "WINDOWID=%d", 
				 (int) GDK_WINDOW_XWINDOW(GTK_WIDGET(term)->window));
	 			env_copy[winid_pos] = buffer;


			 if(!strcmp(command, "sh"))
			 {
				 execle(shell->str, name->str, NULL, env_copy);
				 _exit(127);
			 }
			 else
			 {
				 args[0] = "sh";
		 		 args[1] = "-c";
		 		 args[2] = command;
		 		 args[3] = NULL;

				 /* This ensures that the signals are sent to the correct window 
					* see the previous comment
					*/
				 environ = env_copy;

		 		 execvp(args[0], args);
				 _exit(127);
			 }
     }

     default:
     break;
   }

	 if(new_page)
	 	gtk_notebook_set_page(GTK_NOTEBOOK(app.notebook), -1);


   return 0;
 }


