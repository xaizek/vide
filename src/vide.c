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


#include <sys/param.h>

#if defined(__FreeBSD_version)
	#define RXVT_VIM "xterm -e vim"
	#define RXVT "xterm"
#else
	#define RXVT_VIM "rxvt -e vim"
	#define RXVT "rxvt"
#endif


#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "vide.h"
#include <gdk/gdkx.h>


App app;
Config cfg;
FileView *curr_view;
FileView *other_view;
FileView *command_view;
FileView *filetypes_view;
Bookmarks bk;
Zvt zvt;


Column all_columns[MAX_COLUMNS] =
  { {"Filename", 180, FILENAME, TRUE, name_sort}
,
{"Size", 80, SIZE, TRUE, size_sort}
,
{"Modified", 80, MODIFIED, TRUE, date_sort}
,
{"Accessed", 80, ACCESSED, FALSE, date_sort}
,
{"Changed", 80, CHANGED, FALSE, date_sort}
,
{"Permissions", 80, PERM, TRUE, perm_sort}
,
{"Owner", 60, OWNER, TRUE, user_sort}
,
{"Group", 60, GROUP, FALSE, group_sort}
};

GdkColor EXE_COLOR;
GdkColor DIR_COLOR;
GdkColor LNK_COLOR;
GdkColor DEV_COLOR;
GdkColor COL_COLOR;
GdkColor TAG_COLOR;
GdkColor SOCK_COLOR;
GdkColor DRAG_HILIGHT;
GdkColor CLIST_COLOR;
GdkColor TAB_COLOR;

static void
received_sigchild ()
{
  int status;
	pid_t pid;

  pid = waitpid (-1, &status, WNOHANG);
}

static gint
check_config_files (gpointer data)
{
  struct stat stat_buf;
  gchar filename[PATH_MAX + 10];
  gboolean need_recreate = FALSE;

  g_snprintf (filename, sizeof (filename), "%s/settings", cfg.config_dir);
  if (stat (filename, &stat_buf) == 0)
    {
      if (stat_buf.st_mtime != cfg.config_mtime)
	{
	  status_bar_message ("Configuration files have changed.. reloading");
	  read_config_file ();
	  need_recreate = TRUE;
	  cfg.config_mtime = stat_buf.st_mtime;
	}
    }

  g_snprintf (filename, sizeof (filename), "%s/filetypes", cfg.config_dir);
  if (stat (filename, &stat_buf) == 0)
    {
      if (stat_buf.st_mtime != cfg.filetypes_mtime)
	{
	  status_bar_message ("Configuration files have changed.. reloading");
	  read_filetypes_file ();
	  cfg.filetypes_mtime = stat_buf.st_mtime;
	}
    }

  g_snprintf (filename, sizeof (filename), "%s/bookmarks", cfg.config_dir);
  if (stat (filename, &stat_buf) == 0)
    {
      if (stat_buf.st_mtime != cfg.bookmarks_mtime)
	{
	  status_bar_message ("Configuration files have changed.. reloading");
	  read_bookmarks_file ();
	  need_recreate = TRUE;
	  cfg.bookmarks_mtime = stat_buf.st_mtime;
	}
    }

	/* 
	g_snprintf(filename, sizeof(filename), "%s/term_options", cfg.config_dir);
	if(stat(filename, &stat_buf) == 0)
	{
		if(stat_buf.st_mtime != cfg.term_mtime)
		{
			g_print("in vide.c update term()\n");
			status_bar_message("Configuration files have changed.. reloading");
			read_term_file();
			update_all_terms();
			cfg.term_mtime = stat_buf.st_mtime;
		}
	}
	 */

  g_snprintf (filename, sizeof (filename), "%s/user_commands",
	      cfg.config_dir);

  if (need_recreate)
	{
    recreate_main_window ();
		update_all_terms();
	}

  return TRUE;
}

gint
update_list (gpointer data)
{
  /* check for changes in current directory & other directory 
   * refresh list if any files have changed 
   */

  struct stat statbuf;

	/* This checks if the first page of the notebook is showing.
	 * Updating the clist when it is not on the screen causes errors.
	 */
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(app.notebook)) != 0)
		return TRUE;

  stat (curr_view->dir, &statbuf);
/* if (statbuf.st_mtime != curr_view->dir_mtime) 
    {
			
      GtkAdjustment *vadj;
      gfloat scrollbar_pos;
      gint last_selected_row;


      vadj = gtk_clist_get_vadjustment (GTK_CLIST (curr_view->clist));
      scrollbar_pos = vadj->value;
      last_selected_row = curr_view->row;
      gtk_clist_freeze(GTK_CLIST(curr_view->clist));
      load_dir_list(curr_view);

      gtk_adjustment_set_value(GTK_ADJUSTMENT(vadj), scrollbar_pos);
      gtk_clist_unselect_all(GTK_CLIST(curr_view->clist));

			focus_on_row(curr_view,
					curr_view->row < GTK_CLIST(curr_view->clist)->rows
					? curr_view->row
					: GTK_CLIST(curr_view->clist)->rows);

      gtk_widget_grab_focus(curr_view->clist);
      gtk_clist_thaw(GTK_CLIST(curr_view->clist));
    } 
		*/
  if (statbuf.st_mtime != curr_view->dir_mtime)
  {
    GtkAdjustment *vadj;
    gfloat scrollbar_pos;
    GList *tmp;
		int index[GTK_CLIST(curr_view->clist)->rows];
		int sel = 0;

    vadj = gtk_clist_get_vadjustment(GTK_CLIST(curr_view->clist));
    scrollbar_pos = vadj->value;
    
    tmp = g_list_copy(GTK_CLIST(curr_view->clist)->selection);
    gtk_clist_freeze(GTK_CLIST(curr_view->clist));
    load_dir_list(curr_view);

    gtk_adjustment_set_value(GTK_ADJUSTMENT(vadj), scrollbar_pos);
    gtk_clist_unselect_all(GTK_CLIST(curr_view->clist));
    for (; tmp != NULL; tmp = tmp->next)
    {
      gint i = (gint)tmp->data;
      if (i < GTK_CLIST(curr_view->clist)->rows)
			{
        gtk_clist_select_row(GTK_CLIST(curr_view->clist), i, 0);
				index[sel] = i;
				sel++;
			}
      if (tmp->next == NULL && i < GTK_CLIST(curr_view->clist)->rows)
			{
        GTK_CLIST(curr_view->clist)->focus_row = i;
			}
			else
			{
					GTK_CLIST(curr_view->clist)->focus_row = index[sel];
			}
    }
		/* no rows selected */
		if(!sel)
			focus_on_row(curr_view, GTK_CLIST(curr_view->clist)->rows -1);
    gtk_widget_draw_focus(curr_view->clist);
    gtk_clist_thaw(GTK_CLIST(curr_view->clist));
  } 


  stat(other_view->dir, &statbuf);
  if (statbuf.st_mtime != other_view->dir_mtime)
  {
    load_dir_list(other_view);
    chdir(curr_view->dir);
  }


  return TRUE;
}


int
main (int argc, char *argv[])
{
  struct sigaction new_action;
  gchar *home;
	gchar current_dir[PATH_MAX];
	gint i;

	/* default terminal color scheme */

	gushort red[] = { 0x0000, 0xffff, 0x0000, 0xffff, 0x0000, 0xffff,
		0x0000, 0xffff, 0x0000, 0xffff, 0x0000, 0xffff, 0x0000, 0xffff, 0x0000,
		0xffff, 0x0,    0x0 };
	gushort grn[] = { 0x0000, 0x0000, 0xffff, 0xffff, 0x0000, 0x0000,
		0xffff, 0xffff, 0x0000, 0x0000, 0xffff, 0xffff, 0x0000, 0x0000, 0xffff,
		0xffff, 0x0,    0x0 };
	gushort blu[] = { 0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff,
		0xffff, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff,
		0xffff, 0x0,    0x0 };

	/* white on black foreground/background */
	red[16] = red[7];
	blu[16] = blu[7];
	grn[16] = grn[7];
	red[17] = red[0];
	blu[17] = blu[0];
	grn[17] = blu[0];


	for(i = 0; i < 18; i++)
	{
		zvt.palette[i].red = red[i];
		zvt.palette[i].green = grn[i];
		zvt.palette[i].blue = blu[i];
	}

	cfg.current_term = NULL;

	cfg.term_count = 0;

	/* i18n support */
	gtk_set_locale();

	getcwd(current_dir, sizeof(current_dir));


  /* Set up handler for sigchild so that we don't get zombies */
  new_action.sa_handler = received_sigchild;
  sigemptyset (&new_action.sa_mask);
  new_action.sa_flags = SA_RESTART;
  sigaction (SIGCHLD, &new_action, NULL);

  /*g_thread_init (NULL);*/
  gtk_init (&argc, &argv);

  /* Setup the default colors and font */
  gdk_color_parse ("forest green", &EXE_COLOR);
  gdk_color_parse ("blue", &DIR_COLOR);
  gdk_color_parse ("sky blue", &LNK_COLOR);
  gdk_color_parse ("orange", &DEV_COLOR);
  gdk_color_parse ("purple", &SOCK_COLOR);
  gdk_color_parse ("dark gray", &COL_COLOR);
  gdk_color_parse ("yellow", &TAG_COLOR);
  gdk_color_parse ("wheat", &DRAG_HILIGHT);
	gdk_color_parse ("red", &TAB_COLOR);

	g_snprintf(zvt.term_font, sizeof(zvt.term_font), 
		"-Misc-Fixed-Bold-R-Normal--14-130-75-75-C-70-ISO8859-1");

  cfg.filetypes = NULL;
  cfg.bookmarks = NULL;
	cfg.terms = NULL;
	app.right_view.history = NULL;
	app.left_view.history = NULL;
	app.right_view.menu_history = NULL;
	app.left_view.menu_history = NULL;
	cfg.fileselector = FALSE;
	

	/* this is here in case a previous version of vide configuration file 
	 * exists that doesn't have the cfg.vi_clone or cfg.man_program set
	 */
	strncpy(cfg.man_program, "pauseme man -a", sizeof(cfg.man_program));
	strncpy(cfg.vi_clone, "vim", sizeof(cfg.vi_clone));
  /* Prepare and read config files */
  set_config_dir ();
  if (!read_config_file ())
    {
      /* Set Default values */
			g_snprintf(cfg.start_filter, sizeof(cfg.start_filter), "\\.o$");
			cfg.invert_filter = TRUE;
			cfg.save_position = FALSE;
			cfg.save_position = FALSE;
      cfg.show_hidden = FALSE;
      cfg.start_with_cwd = FALSE; 
			strncpy(cfg.vi_clone, "vim", sizeof(cfg.vi_clone));
			strncpy(cfg.vi_command, "vim", sizeof(cfg.vi_command));
      strncpy (cfg.viewer_command, RXVT_VIM,
	       sizeof (cfg.viewer_command));
      cfg.window_width = 640;
      cfg.window_height = 480;
      cfg.window_xpos = 0;
      cfg.window_ypos = 0;
      cfg.dir_history_max_length = 15; 
      cfg.command_history_max_length = 10;
      strncpy (cfg.xterm_command, RXVT, sizeof (cfg.xterm_command));
      chdir (getenv ("HOME"));
      getcwd (app.left_view.dir, PATH_MAX);
      strncpy (app.right_view.dir, app.left_view.dir, PATH_MAX);
      strncpy (cfg.left_startup_dir, app.left_view.dir, PATH_MAX);
      strncpy (cfg.right_startup_dir, app.left_view.dir, PATH_MAX);
    }

	if(!read_term_file())
	{
		int i;
		gushort rxvt_red[] = { 0x0000, 0xffff, 0x0000, 0xffff, 0x0000,
			0xffff, 0x0000, 0xffff, 0x0000, 0xffff, 0x0000, 0xffff, 0x0000,
			0xffff, 0x0000, 0xffff, 0x0,    0x0 };
		gushort rxvt_grn[] = { 0x0000, 0x0000, 0xffff, 0xffff, 0x0000,
			0x0000, 0xffff, 0xffff, 0x0000, 0x0000, 0xffff, 0xffff, 0x0000,
			0x0000, 0xffff, 0xffff, 0x0,    0x0 };
		gushort rxvt_blu[] = { 0x0000, 0x0000, 0x0000, 0x0000, 0xffff,
			0xffff, 0xffff, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, 0xffff,
			0xffff, 0xffff, 0xffff, 0x0,    0x0 };

		for(i = 0; i < 18; i++)
		{
			zvt.palette[i].red = rxvt_red[i];
			zvt.palette[i].green = rxvt_grn[i];
			zvt.palette[i].blue = rxvt_blu[i];
		}

		zvt.foreground[0].red = rxvt_red[0];
		zvt.foreground[0].green = rxvt_grn[0];
		zvt.foreground[0].blue = rxvt_blu[0];
		zvt.foreground[1].red = rxvt_red[7];
		zvt.foreground[1].green = rxvt_grn[7];
		zvt.foreground[1].blue = rxvt_blu[7];

		/* default zvt terminal options */
		zvt.tab_position = GTK_POS_BOTTOM;
		zvt.terminal_bell = FALSE;
		zvt.use_blinking_cursor = TRUE;
		zvt.scroll_on_keystroke = FALSE;
		zvt.scroll_on_output = FALSE;
		zvt.swap_backspace = FALSE;
		zvt.transparent = FALSE;
		zvt.shaded = FALSE;
		zvt.use_pixmap = FALSE;
		zvt.scroll_lines = 100;
		zvt.scrollbar_position = 0;
		g_snprintf(zvt.background, sizeof(zvt.background), "WhiteOnBlack");
		g_snprintf(zvt.term_color, sizeof(zvt.term_color), "Rxvt");

	}

  if (!read_filetypes_file ())
    {
      /* Setup some default filetypes */
      add_filetype ("jpeg,jpg,png,xpm,gif", "xv,gimp", "Image Files");
      add_filetype ("c,cpp,h,pl,java, py", "vi", "Source Code Files");
      add_filetype ("o,so,a", "x nm %f | less", "Object Files");
      add_filetype ("htm,html,php", "netscape, vi", "HTML Documents");
      add_filetype ("tar.gz,tgz", "x tar xzvf %f ,x tar tzvf %f | less",
		    "Gzipped Tarballs");
			add_filetype("zip, Z", "x unzip %f", "Zip");
			add_filetype ("tar.bz2", "x bzip2 -cd %f | tar tv | less, x bzip2 -cd %f | tar xf -", "Bunzip2");
      add_filetype ("rpm", "x rpm -qlip %f | less,x rpm -Uvh %f | less",
		    "RPM Packages");
      add_filetype ("pdf", "acroread,xpdf", "Adobe Acrobat Documents");
      add_filetype ("ps", "gv", "PostScript Documents");
			add_filetype ("rm, ram", "realplay", "Real Player");
    }

  if (!read_bookmarks_file ())
    {
      /* Setup some default bookmarks */
      if ((home = getenv ("HOME")) != NULL)
				add_bookmark("H", home, "../");
			add_bookmark("R", "/", "../");
			add_bookmark("U", "/usr", "../");
			add_bookmark("L", "/usr/local", "../");
    }


  if (!read_user_commands_file ())
    {
      gchar trash_com[128];
			gchar memo_com[128];
      g_snprintf (trash_com, 128, "mv %%f %s", cfg.trash);
			g_snprintf (memo_com, 128, "!%s %s/memo", cfg.viewer_command,
					cfg.config_dir);

      add_user_command (" ", "NULL");
			add_user_command ("Memo", memo_com);
      add_user_command ("Trash  :d", trash_com);
      add_user_command ("Copy  :co", "cp -R %f %D");
      add_user_command ("Move  :m", "mv %f %D");
			add_user_command ("Rename", "mv %f %{%f New Filename:}");
			add_user_command ("New File", "touch %{Name of New File:}");
			add_user_command ("New Dir", "mkdir %{Name of New Directory:}");

    }

	if(!read_programs_file())
	{
		add_program("Netscape", "netscape");
		add_program("Gimp", "gimp");
		add_program("Glade", "glade");
		add_program("X-chat", "xchat");
	}
	if(!read_command_mode_file())
	{
		add_command("mutt", "!mutt");
		add_command("ps", "!top");
		add_command("vi", "exe rxvt -e vim %f");
		add_command("Backup", "/~$");
	  add_command("touch", "exe touch %{Name of new file.}");
		add_command("vide", "exe vide %d %D");
	}


	if(cfg.start_with_cwd)
	{
		strncpy(app.right_view.dir, current_dir, sizeof(app.right_view.dir));
		strncpy(app.left_view.dir, current_dir, sizeof(app.left_view.dir));
	}
	else
	{
  	if (cfg.right_startup_dir != NULL)
    	strncpy (app.right_view.dir, cfg.right_startup_dir,
	     	sizeof (app.right_view.dir));
  	if (cfg.left_startup_dir != NULL)
    	strncpy (app.left_view.dir, cfg.left_startup_dir,
	     	sizeof (app.left_view.dir));
	}



		if(argv[1] != NULL)
		{
    		strncpy (app.left_view.dir, argv[1], sizeof (app.left_view.dir));

    		if (argv[2] != NULL)
						strncpy (app.right_view.dir, argv[2], sizeof (app.right_view.dir));
		}


	app.left_view.last_row = 0;
	app.right_view.last_row = 0;
	strncpy (app.left_view.last_dir, app.left_view.dir, PATH_MAX);
	strncpy (app.right_view.last_dir, app.right_view.dir, PATH_MAX);



  /* Build GUI */
  create_main_window ();
  initialize_filters (&app.right_view);
  initialize_filters (&app.left_view);
  change_dir(&app.right_view, app.right_view.dir);
  change_dir(&app.left_view, app.left_view.dir);
  sort_list (&app.right_view, name_sort, GTK_SORT_ASCENDING, 0);
  sort_list (&app.left_view, name_sort, GTK_SORT_ASCENDING, 0);
	gtk_widget_grab_focus(app.left_view.clist);
	gtk_clist_select_row(GTK_CLIST(app.left_view.clist), 0, 0);


	gtk_notebook_set_page(GTK_NOTEBOOK(app.notebook), 0);

  /* checks for changes in configuration files */
  gtk_timeout_add (1000, check_config_files, NULL);
  /* checks for changes in current directory  */
  gtk_timeout_add (500, update_list, NULL);

  gtk_main ();

  exit (0);
}
