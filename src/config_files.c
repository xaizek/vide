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


#include<sys/param.h>
#if defined(__FreeBSD_version)
	#define CP_HELP "cp %%PREFIX%%//share/vide/vide%s.txt %s"
#else
	#define CP_HELP "cp /usr/local/share/vide/vide%s.txt %s"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "vide.h"

/*static void
write_data (gpointer data, gpointer file)
{
  fprintf ((FILE *) file, "%s\n", (gchar *) data);
}*/

static void
create_memo_file(gchar *memo_file)
{
	FILE *f;
	gchar memo_text[] = {"
		This is here as an example.  I find it useful.  But you can
			remove it from the command list, if you find it annoying.\n
			"};

	if((f = fopen(memo_file, "w")) == NULL)
	{
		fprintf(stderr, "Unable to create memo file\n");
		return;
	}

	fprintf(stderr, "Creating memo file\n");
	fprintf(f, "%s\n", memo_text);
	fclose(f);
}
			



static void
create_help_file(gchar *help_file)
{
	gchar command[PATH_MAX];

	g_snprintf(command, sizeof(command), CP_HELP,
			VERSION, help_file);
	file_exec(command);

}




void
set_config_dir ()
{
	FILE *f;
	gchar help_file[PATH_MAX];
	gchar memo_file[PATH_MAX];
  gchar *home_dir = getenv ("HOME");

  if (home_dir)
    {
      g_snprintf (cfg.config_dir, sizeof (cfg.config_dir), "%s/.vide",
		  home_dir);
      g_snprintf (cfg.trash, sizeof (cfg.trash), "%s/.vide/Trash", home_dir);
			g_snprintf(help_file, sizeof(help_file), "%s/.vide/vide%s.txt", home_dir,
					VERSION);
			g_snprintf(memo_file, sizeof(memo_file), "%s/.vide/memo", home_dir);

      if (chdir (cfg.config_dir))
			{
	  		fprintf (stderr, "Creating config dir\n");
	  		if (mkdir (cfg.config_dir, 0777))
	    	{
	      	fprintf (stderr, "Couldn't make the config dir\n");
	      	return;
	    	}
				if ((f = fopen(help_file, "r")) == NULL)
				{
					fprintf(stderr, "Unable to open help file\n");
					create_help_file(help_file);
				}
				if ((f = fopen(memo_file, "r")) == NULL)
					create_memo_file(memo_file);

			}
			/* Check if help file is current */
			if((f = fopen(help_file, "r")) == NULL)
						create_help_file(help_file);

      if (chdir (cfg.trash))
			{
	  		if (mkdir (cfg.trash, 0777))
	    		{
	      		fprintf (stderr, "Couldn't make the trash dir\n");
	      		return;
	    		}
			}
    }
  	else
    {
      fprintf (stderr, "HOME environtment variable not set\n");
      return;
    }
}

gboolean
read_term_file()
{
  FILE *f;
  gchar term_file[PATH_MAX + 10];
  gchar line[MAX_LEN];
	gchar *value;

  g_snprintf (term_file, sizeof (term_file), "%s/term_options",
	      cfg.config_dir);
  if ((f = fopen (term_file, "r")) == NULL)
    {
      fprintf (stderr, "Unable to open term options file for reading\n");
			fprintf(stderr, "Will use default settings\n");
      return FALSE;
    }

	while(fgets(line, MAX_LEN, f))
	{
		chomp(line);

		if(strstr(line, "TERMINAL_BELL"))
		{
			value = (strchr(line, '=') +1);
			zvt.terminal_bell = STREQ(value, "TRUE") ? TRUE : FALSE;
			continue;
		}
		if(strstr(line, "USE_BLINKING_CURSOR"))
		{
			value = (strchr(line, '=') +1);
			zvt.use_blinking_cursor = STREQ(value, "TRUE") ? TRUE : FALSE;
			continue;
		}
		if(strstr(line, "SCROLL_ON_KEYSTROKE"))
		{
			value = (strchr(line, '=') +1);
			zvt.scroll_on_keystroke = STREQ(value, "TRUE") ? TRUE : FALSE;
			continue;
		}
		if(strstr(line, "SCROLL_ON_OUTPUT"))
		{
			value = (strchr(line, '=') +1);
			zvt.scroll_on_output = STREQ(value, "TRUE") ? TRUE : FALSE;
			continue;
		}
		if(strstr(line, "SWAP_BACKSPACE"))
		{
			value = (strchr(line, '=') +1);
			zvt.swap_backspace = STREQ(value, "TRUE") ? TRUE : FALSE;
			continue;
		}
		if(strstr(line, "TRANSPARENT"))
		{
			value = (strchr(line, '=') +1);
			zvt.transparent = STREQ(value, "TRUE") ? TRUE : FALSE;
			continue;
		}
		if(strstr(line, "SHADED"))
		{
			value = (strchr(line, '=') +1);
			zvt.shaded = STREQ(value, "TRUE") ? TRUE : FALSE;
			continue;
		}
		if(strstr(line, "USE_PIXMAP"))
		{
			value = (strchr(line, '=') +1);
			zvt.use_pixmap = STREQ(value, "TRUE") ? TRUE : FALSE;
			continue;
		}
		if(strstr(line, "PIXMAP"))
		{
			value = (strchr(line, '=') +1);
	  	strncpy (zvt.pixmap, value, sizeof (zvt.pixmap));
			continue;
		}
		if(strstr(line, "SCROLLBAR_POSITION"))
		{
			value = (strchr(line, '=') +1);
			zvt.scrollbar_position = STREQ(value, "TRUE") ? TRUE : FALSE;
			continue;
		}
		if(strstr(line, "TAB_POSITION"))
		{
			value = (strchr(line, '=') +1);
			zvt.tab_position = atoi(value);
			continue;
		}
		if(strstr(line, "TERM_COLOR"))
		{
			value = (strchr(line, '=') +1);
			strncpy(zvt.term_color, value, sizeof(zvt.term_color));
			continue;
		}
		if(strstr(line, "BACKGROUND"))
		{
			value = (strchr(line, '=') +1);
			strncpy(zvt.background, value, sizeof(zvt.background));
			continue;
		}
		if(strstr(line, "SCROLL_LINES"))
		{
			value = (strchr(line, '=') +1);
			zvt.scroll_lines = atof(value);
			continue;
		}
		if(strstr(line, "TERM_FONT"))
		{
			value = (strchr(line, '=') +1);
			strncpy(zvt.term_font, value, sizeof(zvt.term_font));
			continue;
		}
		if(strstr(line, "SHOW_TABS"))
		{
			value = (strchr(line, '=') +1);
			zvt.show_tabs = STREQ(value, "TRUE") ? TRUE : FALSE;
			continue;
		}
		if(strstr(line, "RED="))
		{
			gint x = 0;
			value = (strchr(line, '=') +1);
			x = atoi(value);
			value++;
			value = (strchr(value, '=') +1);
			zvt.palette[x].red = atoi(value);
			continue;
			
		}
		if(strstr(line, "GREEN="))
		{
			gint x = 0;
			value = (strchr(line, '=') +1);
			x = atoi(value);
			value++;
			value = (strchr(value, '=') +1);
			zvt.palette[x].green = atoi(value);
			continue;
		}
		if(strstr(line, "BLUE="))
		{
			gint x = 0;
			value = (strchr(line, '=') +1);
			x = atoi(value);
			value++;
			value = (strchr(value, '=') +1);
			zvt.palette[x].blue = atoi(value);
			continue;
		}
		if(strstr(line, "REDFORE"))
		{
			value = (strchr(line, '=') +1);
			zvt.foreground[0].red = atoi(value);
			continue;
		}
		if(strstr(line, "GREENFORE"))
		{
			value = (strchr(line, '=') +1);
			zvt.foreground[0].green = atoi(value);
			continue;
		}
		if(strstr(line, "BLUEFORE"))
		{
			value = (strchr(line, '=') +1);
			zvt.foreground[0].blue = atoi(value);
			continue;
		}
		if(strstr(line, "REDBACK"))
		{
			value = (strchr(line, '=') +1);
			zvt.foreground[1].red = atoi(value);
			continue;
		}
		if(strstr(line, "GREENBACK"))
		{
			value = (strchr(line, '=') +1);
			zvt.foreground[1].green = atoi(value);
			continue;
		}
		if(strstr(line, "BLUEBACK"))
		{
			value = (strchr(line, '=') +1);
			zvt.foreground[1].blue = atoi(value);
			continue;
		}

	}
	return TRUE;
}

void
write_term_file()
{
  FILE *f;
  gchar term_file[PATH_MAX + 10];
  struct stat stat_buf;
	int i;

  g_snprintf (term_file, sizeof (term_file),
	      "%s/term_options", cfg.config_dir);

  if ((f = fopen (term_file, "w")) == NULL)
    {
      fprintf (stderr, "Unable to open term_options file for write\n");
      return;
    }

	fprintf(f, "TERMINAL_BELL=%s\n", (zvt.terminal_bell ? "TRUE" : "FALSE"));
	fprintf(f, "USE_BLINKING_CURSOR=%s\n", (zvt.use_blinking_cursor ?
			"TRUE" : "FALSE"));
	fprintf(f, "SCROLL_ON_KEYSTROKE=%s\n", (zvt.scroll_on_keystroke ? "TRUE" :
				"FALSE"));
	fprintf(f, "SCROLL_ON_OUTPUT=%s\n", (zvt.scroll_on_output ? "TRUE" :
				"FALSE"));
	fprintf(f, "SWAP_BACKSPACE=%s\n", (zvt.swap_backspace ? "TRUE" : "FALSE"));
	fprintf(f, "TRANSPARENT=%s\n", (zvt.transparent ? "TRUE" : "FALSE"));
	fprintf(f, "SHADED=%s\n", (zvt.shaded ? "TRUE" : "FALSE"));
	fprintf(f, "USE_PIXMAP=%s\n", (zvt.use_pixmap ? "TRUE" : "FALSE"));
	fprintf(f, "PIXMAP=%s\n", zvt.pixmap);
	fprintf(f, "SCROLLBAR_POSITION=%d\n", zvt.scrollbar_position);
	fprintf(f, "TAB_POSITION=%d\n", zvt.tab_position);
	fprintf(f, "TERM_COLOR=%s\n", zvt.term_color);
	fprintf(f, "BACKGROUND=%s\n", zvt.background);
	fprintf(f, "SCROLL_LINES=%f\n", zvt.scroll_lines);
	fprintf(f, "TERM_FONT=%s\n", zvt.term_font);
	fprintf(f, "SHOW_TABS=%s\n", (zvt.show_tabs ? "TRUE" : "FALSE"));

	for(i = 0; i < 18; i++)
	{
		fprintf(f, "RED=%d=%d\n", i, zvt.palette[i].red);
		fprintf(f, "GREEN=%d=%d\n", i, zvt.palette[i].green);
		fprintf(f, "BLUE=%d=%d\n", i, zvt.palette[i].blue);
	}

	fprintf(f, "REDFORE=%d\n", zvt.foreground[0].red);
	fprintf(f, "GREENFORE=%d\n", zvt.foreground[0].green);
	fprintf(f, "BLUEFORE=%d\n", zvt.foreground[0].blue);
	fprintf(f, "REDBACK=%d\n", zvt.foreground[1].red);
	fprintf(f, "GREENBACK=%d\n", zvt.foreground[1].green);
	fprintf(f, "BLUEBACK=%d\n", zvt.foreground[1].blue);

	  fclose (f);

  if (stat (term_file, &stat_buf) != 0)
    fprintf (stderr, "Unable to stat term options file.. this is wierd\n");
  else
    cfg.term_mtime = stat_buf.st_mtime;
}

void
write_filetypes_file ()
{
  FILE *f;
  gchar filetypes_file[PATH_MAX + 10];
  GList *tmp = NULL;
  struct stat stat_buf;

  g_snprintf (filetypes_file, sizeof (filetypes_file),
	      "%s/filetypes", cfg.config_dir);

  if ((f = fopen (filetypes_file, "w")) == NULL)
    {
      fprintf (stderr, "Unable to open filetypes file for write\n");
      return;
    }

  for (tmp = cfg.filetypes; tmp != NULL; tmp = tmp->next)
    {
      FileType *ft = tmp->data;

      fprintf (f, "%s;%s;%s\n", ft->description, ft->extensions,
	       ft->programs);
    }
  fclose (f);

  if (stat (filetypes_file, &stat_buf) != 0)
    fprintf (stderr, "Unable to stat filetypes file.. this is wierd\n");
  else
    cfg.filetypes_mtime = stat_buf.st_mtime;
	g_list_free(tmp);
}


gboolean read_filetypes_file ()
{
  FILE *f;
  struct stat stat_buf;
  gchar filetypes_file[PATH_MAX + 10];
  gchar line[MAX_LEN];
  gchar *extensions;
  gchar *programs;
  gchar *description;
  gchar *s;

  g_snprintf (filetypes_file, sizeof (filetypes_file), "%s/filetypes",
	      cfg.config_dir);
  if ((f = fopen (filetypes_file, "r")) == NULL)
    {
      fprintf (stderr, "Unable to open filetypes file for reading\n");
			fprintf(stderr, "Will use default settings\n");
      return FALSE;
    }

	g_list_free(cfg.filetypes);
  cfg.filetypes = NULL;
  while (fgets (line, MAX_LEN, f))
    {
      if (line[0] == '\n')
			continue;

      if ((s = strstr (line, ";")) != NULL)
				*s = '\0';
      else
				continue;

      description = strdup (line);
      s++;
      extensions = s;

      if ((s = strstr (extensions, ";")) != NULL)
				*s = '\0';
      else
				continue;
      extensions = strdup (extensions);
      s++;
      programs = s;

      chomp (programs);

      add_filetype (extensions, programs, description);

      free (description);
      free (extensions);
    }
  fclose (f);

  if (stat (filetypes_file, &stat_buf) != 0)
    fprintf (stderr, "Unable to stat filetypes file.. this is wierd\n");
  else
    cfg.filetypes_mtime = stat_buf.st_mtime;

  return TRUE;
}

void
write_bookmarks_file ()
{
  FILE *f;
  struct stat stat_buf;
  gchar filename[PATH_MAX + NAME_MAX];
  GList *tmp;

  g_snprintf (filename, sizeof (filename), "%s/bookmarks",
	      cfg.config_dir);
  if ((f = fopen (filename, "w")) == NULL)
    {
      fprintf (stderr, "Unable to open bookmarks file for writing\n");
      return;
    }

  for (tmp = cfg.bookmarks; tmp != NULL; tmp = tmp->next)
    {
      Bookmarks *bk = tmp->data;

      fprintf (f, "%s=%s=%s\n", bk->mark, bk->dir, bk->file);
    }
  fclose (f);

  if (stat (filename, &stat_buf) != 0)
    fprintf (stderr, "Unable to stat bookmarks file.. this is wierd\n");
  else
    cfg.bookmarks_mtime = stat_buf.st_mtime;
	g_list_free(tmp);
}
gboolean read_bookmarks_file ()
{
  FILE *f;
  struct stat stat_buf;
  gchar filename[PATH_MAX + NAME_MAX];
  gchar line[MAX_LEN], *s;

  g_snprintf (filename, sizeof (filename), "%s/bookmarks",
	      cfg.config_dir);
  if ((f = fopen (filename, "r")) == NULL)
    {
      fprintf (stderr, "Unable to open bookmarks file for reading\n");
			fprintf(stderr, "Will use default settings\n");

      return FALSE;
    }

	g_list_free(cfg.bookmarks);
  cfg.bookmarks = NULL;
  while (fgets (line, MAX_LEN, f))
    {
      if (line[0] == '\n')
				continue;

      if ((s = strtok (line, "=")) != NULL)
      	strncpy (bk.mark, s, sizeof (bk.mark));
			if((s = strtok(NULL, "=")) != NULL)
      	strncpy (bk.dir, s, sizeof (bk.dir));
			if((s = strtok(NULL, "\n")) != NULL)
				strncpy (bk.file, s, sizeof(bk.file));

      cfg.bookmarks = g_list_append (cfg.bookmarks,
					 duplicate (&bk,
						    sizeof (Bookmarks)));
    }
  fclose (f);

  if (stat (filename, &stat_buf) != 0)
    fprintf (stderr, "Unable to stat bookmarks file.. this is wierd\n");
  else
    cfg.bookmarks_mtime = stat_buf.st_mtime;

  return TRUE;
}
gboolean read_user_commands_file ()
{
  FILE *f;
  struct stat stat_buf;
  gchar filename[PATH_MAX + NAME_MAX];
  gchar line[MAX_LEN], *s;
  UserCommand command;

  g_snprintf (filename, sizeof (filename), "%s/user_commands",
	      cfg.config_dir);
  if ((f = fopen (filename, "r")) == NULL)
    {
      fprintf (stderr, "Unable to open user_commands file for reading\n");
			fprintf(stderr, "Will use default settings\n");

      return FALSE;
    }

	g_list_free(cfg.user_commands);
  cfg.user_commands = NULL;
  while (fgets (line, MAX_LEN, f))
    {
      if (line[0] == '\n')
				continue;

      if ((s = strchr (line, '=')) != NULL)
				*s++ = '\0';
      else
				continue;

      strncpy (command.name, line, sizeof (command.name));
      chomp (s);
      strncpy (command.action, s, sizeof (command.action));

      cfg.user_commands = g_list_append (cfg.user_commands,
					 duplicate (&command,
						    sizeof (UserCommand)));
    }
  fclose (f);

  if (stat (filename, &stat_buf) != 0)
    fprintf (stderr, "Unable to stat user_commands file.. this is wierd\n");
  else
    cfg.user_commands_mtime = stat_buf.st_mtime;

  return TRUE;
}

void
write_user_commands_file ()
{
  FILE *f;
  struct stat stat_buf;
  gchar filename[PATH_MAX + NAME_MAX];
  GList *tmp;

  g_snprintf (filename, sizeof (filename), "%s/user_commands",
	      cfg.config_dir);
  if ((f = fopen (filename, "w")) == NULL)
    {
      fprintf (stderr, "Unable to open user_commands file for writing\n");
      return;
    }

  for (tmp = cfg.user_commands; tmp != NULL; tmp = tmp->next)
    {
      UserCommand *command = tmp->data;

      fprintf (f, "%s=%s\n", command->name, command->action);
    }
  fclose (f);

  if (stat (filename, &stat_buf) != 0)
    fprintf (stderr, "Unable to stat user_commands file.. this is wierd\n");
  else
    cfg.user_commands_mtime = stat_buf.st_mtime;
	g_list_free(tmp);
}

 /* programs  */
void
write_programs_file ()
{
  FILE *f;
  struct stat stat_buf;
  gchar filename[PATH_MAX + NAME_MAX];
  GList *tmp;

  g_snprintf (filename, sizeof (filename), "%s/programs",
	      cfg.config_dir);
  if ((f = fopen (filename, "w")) == NULL)
    {
      fprintf (stderr, "Unable to open programs file for writing\n");
			fprintf(stderr, "Will use default settings\n");

      return;
    }

  for (tmp = cfg.programs; tmp != NULL; tmp = tmp->next)
    {
      Programs *program = tmp->data;

      fprintf (f, "%s=%s\n", program->name, program->action);
    }
  fclose (f);

  if (stat (filename, &stat_buf) != 0)
    fprintf (stderr, "Unable to stat programs file.. this is wierd\n");
  else
    cfg.programs_mtime = stat_buf.st_mtime;
	g_list_free(tmp);
}

gboolean read_programs_file ()
{
  FILE *f;
  struct stat stat_buf;
  gchar filename[PATH_MAX + NAME_MAX];
  gchar line[MAX_LEN], *s;
  Programs program;

  g_snprintf (filename, sizeof (filename), "%s/programs",
	      cfg.config_dir);
  if ((f = fopen (filename, "r")) == NULL)
    {
      fprintf (stderr, "Unable to open programs file for reading\n");
			fprintf(stderr, "Will use default settings\n");

      return FALSE;
    }

	g_list_free(cfg.programs);
  cfg.programs = NULL;
  while (fgets (line, MAX_LEN, f))
    {
      if (line[0] == '\n')
				continue;

      if ((s = strchr (line, '=')) != NULL)
				*s++ = '\0';
      else
				continue;

      strncpy (program.name, line, sizeof (program.name));
      chomp (s);
      strncpy (program.action, s, sizeof (program.action));

      cfg.programs = g_list_append (cfg.programs,
					 duplicate (&program,
						    sizeof (Programs)));
    }
  fclose (f);

  if (stat (filename, &stat_buf) != 0)
    fprintf (stderr, "Unable to stat programs file.. this is wierd\n");
  else
    cfg.programs_mtime = stat_buf.st_mtime;

  return TRUE;
}

/* command_mode_programs */
void
write_command_mode_file ()
{
  FILE *f;
  struct stat stat_buf;
  gchar filename[PATH_MAX + NAME_MAX];
  GList *tmp;

  g_snprintf (filename, sizeof (filename), "%s/command_mode",
	      cfg.config_dir);
  if ((f = fopen (filename, "w")) == NULL)
    {
      fprintf (stderr, "Unable to open command_mode file for writing\n");
			fprintf(stderr, "Will use default settings\n");

      return;
    }

  for (tmp = cfg.command_mode_programs; tmp != NULL; tmp = tmp->next)
    {
      Command_Mode_Programs *command_mode_programs = tmp->data;

      fprintf (f, "%s=%s\n", command_mode_programs->name,
					command_mode_programs->action);
    }
  fclose (f);

  if (stat (filename, &stat_buf) != 0)
    fprintf (stderr, "Unable to stat programs file.. this is wierd\n");
  else
    cfg.command_mode_programs_mtime = stat_buf.st_mtime;
	g_list_free(tmp);
}


gboolean read_command_mode_file()
{
  FILE *f;
  struct stat stat_buf;
  gchar filename[PATH_MAX + NAME_MAX];
  gchar line[MAX_LEN], *s;
  Command_Mode_Programs command_mode_programs;

  g_snprintf (filename, sizeof (filename), "%s/command_mode",
	      cfg.config_dir);
  if ((f = fopen (filename, "r")) == NULL)
    {
      fprintf (stderr, "Unable to open command_mode file for reading\n");
			fprintf(stderr, "Will use default settings\n");

      return FALSE;
    }

	g_list_free(cfg.command_mode_programs);
  cfg.command_mode_programs = NULL;
  while (fgets (line, MAX_LEN, f))
    {
      if (line[0] == '\n')
				continue;

      if ((s = strchr (line, '=')) != NULL)
				*s++ = '\0';
      else
				continue;

      strncpy (command_mode_programs.name, line, sizeof (command_mode_programs.name));
      chomp (s);
      strncpy (command_mode_programs.action, s, sizeof (command_mode_programs.action));

      cfg.command_mode_programs = g_list_append (cfg.command_mode_programs,
					 duplicate (&command_mode_programs,
						    sizeof (Command_Mode_Programs)));
    }
  fclose (f);

  if (stat (filename, &stat_buf) != 0)
    fprintf (stderr, "Unable to stat programs file.. this is wierd\n");
  else
    cfg.command_mode_programs_mtime = stat_buf.st_mtime;

  return TRUE;
}
void
write_config_file ()
{
  FILE *f;
  struct stat stat_buf;
  gchar config_file[PATH_MAX + 10];
  int i;

  g_snprintf (config_file, sizeof (config_file), "%s/settings",
	      cfg.config_dir);
  if ((f = fopen (config_file, "w")) == NULL)
    {
      fprintf (stderr, "Unable to open config file for writing\n");
      return;
    }

	fprintf(f, "SAVE_POSITION=%s\n", cfg.save_position ? "TRUE" : "FALSE");
	fprintf(f, "START_FILTER=%s\n", cfg.start_filter);
	fprintf(f, "INVERT_FILTER=%s\n", (cfg.invert_filter ? "TRUE" : "FALSE"));
//	fprintf (f, "USE_VI_SERVER=%s\n", (cfg.use_vi_server ? "TRUE" : "FALSE"));
//	fprintf(f, "USE_VIMCLIENT=%s\n", (cfg.use_vimclient ? "TRUE" : "FALSE"));
  fprintf (f, "SHOW_HIDDEN=%s\n", (cfg.show_hidden ? "TRUE" : "FALSE"));
  //fprintf (f, "CONFIRM_DELETE=%s\n", (cfg.confirm_delete ? "TRUE" : "FALSE"));
  //fprintf (f, "CONFIRM_OVERWRITE=%s\n", (cfg.confirm_overwrite
	//				 ? "TRUE" : "FALSE"));
  fprintf (f, "VIEWER_COMMAND=%s\n", cfg.viewer_command);
  fprintf (f, "XTERM_COMMAND=%s\n", cfg.xterm_command);
  fprintf (f, "LEFT_DIR=%s\n", cfg.left_startup_dir);
  fprintf (f, "RIGHT_DIR=%s\n", cfg.right_startup_dir);
  fprintf (f, "START_WITH_CWD=%s\n", (cfg.start_with_cwd
					    ? "TRUE" : "FALSE"));
	fprintf (f, "USE_SCROLLBARS=%s\n", (cfg.use_scrollbars ? "TRUE" : "FALSE"));
	fprintf (f, "VI_CLONE=%s\n", cfg.vi_clone);
	fprintf( f, "VI_COMMAND=%s\n", cfg.vi_command);
  fprintf (f, "DIR_HISTORY=%d\n", cfg.dir_history_max_length);
  fprintf (f, "COMMAND_HISTORY=%d\n", cfg.command_history_max_length);
  //fprintf (f, "DIVIDE_POPUP_MENU=%s\n", (cfg.divide_popup_menu
					 //? "TRUE" : "FALSE"));
  fprintf (f, "WINDOW_HEIGHT=%d\n", app.main_window->allocation.height);
  fprintf (f, "WINDOW_WIDTH=%d\n", app.main_window->allocation.width);
  gdk_window_get_root_origin (app.main_window->window, &cfg.window_xpos,
			      &cfg.window_ypos);
  fprintf (f, "WINDOW_XPOS=%d\n", cfg.window_xpos);
  fprintf (f, "WINDOW_YPOS=%d\n", cfg.window_ypos);
  //fprintf (f, "OUTPUT_TEXT_HIDDEN=%s\n", (cfg.output_text_hidden
	//				  ? "TRUE" : "FALSE"));
  for (i = 0; i < MAX_COLUMNS; i++)
    {
      if (all_columns[i].is_visible)
			{
	  		fprintf (f, "COLUMN=%d\n", i);
			}
    }

  fprintf (f, "COL_COLOR=%04X,%04X,%04X\n", COL_COLOR.red, COL_COLOR.green,
	   COL_COLOR.blue);
  fprintf (f, "DIR_COLOR=%04X,%04X,%04X\n", DIR_COLOR.red, DIR_COLOR.green,
	   DIR_COLOR.blue);
  fprintf (f, "DEV_COLOR=%04X,%04X,%04X\n", DEV_COLOR.red, DEV_COLOR.green,
	   DEV_COLOR.blue);
  fprintf (f, "EXE_COLOR=%04X,%04X,%04X\n", EXE_COLOR.red, EXE_COLOR.green,
	   EXE_COLOR.blue);
  fprintf (f, "LNK_COLOR=%04X,%04X,%04X\n", LNK_COLOR.red, LNK_COLOR.green,
	   LNK_COLOR.blue);
  fprintf (f, "SOCK_COLOR=%04X,%04X,%04X\n", SOCK_COLOR.red, SOCK_COLOR.green,
	   SOCK_COLOR.blue);
  fprintf (f, "TAG_COLOR=%04X,%04X,%04X\n", TAG_COLOR.red, TAG_COLOR.green,
	   TAG_COLOR.blue);
  fprintf (f, "TAB_COLOR=%04X,%04X,%04X\n", TAB_COLOR.red, TAB_COLOR.green,
	   TAB_COLOR.blue);
	for(i = 0; i < 18; i++)
	{
		fprintf(f, "ZVT_COLOR=%d,%04X,%04X,%04X\n", i, zvt.palette[i].red, 
				zvt.palette[i].green, zvt.palette[i].blue);
	}
	fprintf(f, "MAN_PROGRAM=%s\n", cfg.man_program);

  fclose (f);

  if (stat (config_file, &stat_buf) != 0)
    fprintf (stderr, "Unable to stat config file.. this is wierd\n");
  else
    cfg.config_mtime = stat_buf.st_mtime;
}

gboolean read_config_file ()
{
  FILE *f;
  struct stat stat_buf;
  gchar config_file[PATH_MAX + 10];
  gchar line[MAX_LEN];
  gchar *value;
  int clear_default_columns = TRUE;

  g_snprintf (config_file, sizeof (config_file), "%s/settings",
	      cfg.config_dir);
  if ((f = fopen (config_file, "r")) == NULL)
    {
      fprintf (stderr, "Unable to open config file for reading\n");
			fprintf(stderr, "Will use default settings\n");
      return FALSE;
    }

  while (fgets (line, MAX_LEN, f))
    {
      chomp (line);

			if(strstr(line, "SAVE_POSITION"))
			{
				value = (strchr(line, '=') +1);
				cfg.save_position = STREQ(value, "TRUE") ? TRUE : FALSE;
				continue;
			}
			if(strstr(line, "INVERT_FILTER"))
			{
				value = (strchr(line, '=') +1);
				cfg.invert_filter = STREQ(value, "TRUE") ? TRUE : FALSE;
				continue;
			}
			if(strstr(line, "START_FILTER"))
			{
				value = strchr(line, '=') +1;
				g_snprintf(cfg.start_filter, sizeof(cfg.start_filter), "%s", value);
				continue;
			}
			if(strstr (line, "VI_CLONE"))
			{
				value = (strchr(line, '=') +1);
				strncpy(cfg.vi_clone, value, sizeof(cfg.vi_clone));
				continue;
			}
			if(strstr(line, "VI_COMMAND"))
			{
				value = (strchr(line, '=') +1);
				strncpy(cfg.vi_command, value, sizeof(cfg.vi_command));
				continue;
			}
      if (strstr (line, "SHOW_HIDDEN"))
			{
	  		value = (strchr (line, '=') + 1);
	  		cfg.show_hidden = STREQ (value, "TRUE") ? TRUE : FALSE;
	  		continue;
			}
      if (strstr (line, "LEFT_DIR"))
			{
	  		value = (strchr (line, '=') + 1);
	  		strncpy (cfg.left_startup_dir, value,
		   	sizeof (cfg.left_startup_dir));
	  		continue;
			}

      if (strstr (line, "RIGHT_DIR"))
			{
	  		value = (strchr (line, '=') + 1);
	  		strncpy (cfg.right_startup_dir, value,
		   	sizeof (cfg.right_startup_dir));
	  		continue;
			}

      if (strstr (line, "START_WITH_CWD"))
			{
	  		value = (strchr (line, '=') + 1);
	  		cfg.start_with_cwd = STREQ (value, "TRUE") ? TRUE : FALSE;
	  		continue;
			}
			if (strstr (line, "USE_SCROLLBARS"))
			{
				value = (strchr (line, '=') + 1);
				cfg.use_scrollbars = STREQ (value, "TRUE") ? TRUE : FALSE	;
				continue;
			}

      if (strstr (line, "DIR_HISTORY"))
			{
	  		value = (strchr (line, '=') + 1);
	  		cfg.dir_history_max_length = atoi (value);
	  		continue;
			}

      if (strstr (line, "COMMAND_HISTORY"))
			{
	  		value = (strchr (line, '=') + 1);
	  		cfg.command_history_max_length = atoi (value);
	  		continue;
			}

			/*
      if (strstr (line, "DIVIDE_POPUP_MENU"))
			{
	  		value = (strchr (line, '=') + 1);
	  		cfg.divide_popup_menu = STREQ (value, "TRUE") ? TRUE : FALSE;
	  		continue;
			}
			*/


      if (strstr (line, "WINDOW_HEIGHT"))
			{
	  		value = (strchr (line, '=') + 1);
	  		cfg.window_height = atoi (value);
	  		continue;
			}

      if (strstr (line, "WINDOW_WIDTH"))
			{
	  		value = (strchr (line, '=') + 1);
	  		cfg.window_width = atoi (value);
	  		continue;
			}

      if (strstr (line, "WINDOW_XPOS"))
			{
	  		value = (strchr (line, '=') + 1);
	  		cfg.window_xpos = atoi (value);
	  		continue;
			}
      if (strstr (line, "WINDOW_YPOS"))
			{
	  		value = (strchr (line, '=') + 1);
	  		cfg.window_ypos = atoi (value);
	  		continue;
			}
      if (strstr (line, "VIEWER_COMMAND"))
			{
	  		value = (strchr (line, '=') + 1);
	  		strncpy (cfg.viewer_command, value, sizeof (cfg.viewer_command));
	  		continue;
			}
      if (strstr (line, "XTERM_COMMAND"))
			{
	  		value = (strchr (line, '=') + 1);
	  		strncpy (cfg.xterm_command, value, sizeof (cfg.xterm_command));
	  		continue;
			}

			/*
      if (strstr (line, "OUTPUT_TEXT_HIDDEN"))
			{
	  		value = (strchr (line, '=') + 1);
	  		cfg.output_text_hidden = STREQ (value, "TRUE") ? TRUE : FALSE;
	  		continue;
			}
			*/

      if (strstr (line, "COLUMN"))
			{
	  		int i;
	  		if (clear_default_columns)
	    	{
	      	for (i = 0; i < MAX_COLUMNS; i++)
					all_columns[i].is_visible = FALSE;
	      	clear_default_columns = FALSE;
	    	}

	  		value = (strchr (line, '=') + 1);
	  		i = atoi (value);
	  		all_columns[i].is_visible = TRUE;
			}

      if (strstr (line, "COL_COLOR"))
			{
	  		unsigned int red, green, blue;

	  		value = (strchr (line, '=') + 1);
	  		sscanf (value, "%04X,%04X,%04X", &red, &green, &blue);
	  		COL_COLOR.red = red;
	  		COL_COLOR.green = green;
	  		COL_COLOR.blue = blue;
			}

      if (strstr (line, "DIR_COLOR"))
			{
	  		unsigned int red, green, blue;

	  		value = (strchr (line, '=') + 1);
	  		sscanf (value, "%04X,%04X,%04X", &red, &green, &blue);
	  		DIR_COLOR.red = red;
	  		DIR_COLOR.green = green;
	  		DIR_COLOR.blue = blue;
			}

      if (strstr (line, "DEV_COLOR"))
			{
	  		unsigned int red, green, blue;

	  		value = (strchr (line, '=') + 1);
	  		sscanf (value, "%04X,%04X,%04X", &red, &green, &blue);
	  		DEV_COLOR.red = red;
	  		DEV_COLOR.green = green;
	  		DEV_COLOR.blue = blue;
			}

      if (strstr (line, "EXE_COLOR"))
			{
	  		unsigned int red, green, blue;

	  		value = (strchr (line, '=') + 1);
	  		sscanf (value, "%04X,%04X,%04X", &red, &green, &blue);
	  		EXE_COLOR.red = red;
	  		EXE_COLOR.green = green;
	  		EXE_COLOR.blue = blue;
			}

      if (strstr (line, "LNK_COLOR"))
			{
	  		unsigned int red, green, blue;

	  		value = (strchr (line, '=') + 1);
	  		sscanf (value, "%04X,%04X,%04X", &red, &green, &blue);
	  		LNK_COLOR.red = red;
	  		LNK_COLOR.green = green;
	 		  LNK_COLOR.blue = blue;
			}

      if (strstr (line, "SOCK_COLOR"))
			{
	  		unsigned int red, green, blue;

	  		value = (strchr (line, '=') + 1);
	  		sscanf (value, "%04X,%04X,%04X", &red, &green, &blue);
	  		SOCK_COLOR.red = red;
	  		SOCK_COLOR.green = green;
	  		SOCK_COLOR.blue = blue;
			}

      if (strstr (line, "TAG_COLOR"))
			{
	  		unsigned int red, green, blue;

	  		value = (strchr (line, '=') + 1);
	  		sscanf (value, "%04X,%04X,%04X", &red, &green, &blue);
	  		TAG_COLOR.red = red;
	  		TAG_COLOR.green = green;
	  		TAG_COLOR.blue = blue;
			}
      if (strstr (line, "TAB_COLOR"))
			{
	  		unsigned int red, green, blue;

	  		value = (strchr (line, '=') + 1);
	  		sscanf (value, "%04X,%04X,%04X", &red, &green, &blue);
	  		TAB_COLOR.red = red;
	  		TAB_COLOR.green = green;
	  		TAB_COLOR.blue = blue;
			}
			if(strstr(line, "ZVT_COLOR"))
			{
				unsigned int red, green, blue;
				int i;

				value = (strchr(line, '=') +1);
				sscanf(value, "%d,%04X,%04X,%04X", &i, &red, &green, &blue);
				zvt.palette[i].red = red;
				zvt.palette[i].green = green;
				zvt.palette[i].blue = blue;
			}
			if(strstr(line, "MAN_PROGRAM"))
			{
				value = (strchr(line, '=') +1);
	  		strncpy (cfg.man_program, value, sizeof (cfg.man_program));
			}

    }
  fclose (f);

  if (stat (config_file, &stat_buf) != 0)
    fprintf (stderr, "Unable to stat config file.. this is wierd\n");
  else
    cfg.config_mtime = stat_buf.st_mtime;

  return TRUE;
}

