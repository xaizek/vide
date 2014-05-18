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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include "vide.h"

#define FILE_TYPE_LENGTH  64	/* max length of the file type string */

static void
close_cb (GtkWidget * widget, GtkWidget * dialog)
{
  gtk_grab_remove (dialog);
  gtk_widget_destroy (dialog);
	if(1 < gtk_main_level())
		gtk_main_quit();

  curr_view->iter = curr_view->iter->next;
  if (curr_view->iter != NULL)
    {
      FileInfo *info = curr_view->iter->data;
      create_file_info_dialog (info);
    }
  else
    {
      g_list_free (curr_view->iter_base);
      gtk_clist_select_row (GTK_CLIST (curr_view->clist), curr_view->row, 0);
      gtk_widget_grab_focus (curr_view->clist);
    }
}

static void
key_press_cb (GtkWidget * widget, GdkEventKey * event, gpointer data)
{
  if (event->keyval == GDK_Escape)
    close_cb (NULL, widget);
}

static void
get_file_type_using_file_command (gchar * filename, gchar * string, gint len)
{
  FILE *pipe;
  gchar command[MAX_LEN];
  gchar buf[len + NAME_MAX], *s;

  g_snprintf (command, sizeof (command), "file \"%s\"", filename);
  if ((pipe = popen (command, "r")) == NULL)
    {
      fprintf (stderr, "Unable to open pipe for file command\n");
      return;
    }

  fgets (buf, sizeof (buf), pipe);
  if ((s = strchr (buf, ':')) != NULL)
    strncpy (string, s + 2, len);
  else
    strncpy (string, buf, len);

  pclose (pipe);
}

void
create_file_info_dialog (FileInfo * info)
{
  GtkWidget *file_info_dialog;
  GtkWidget *dialog_vbox;
  GtkWidget *action_area;
  GtkWidget *table;
  gchar label_text[MAX_LEN];
  gchar perm_string[11];
  struct tm *tm_ptr;
  struct passwd *pw_buf;
  struct group *grp_buf;

  file_info_dialog = gtk_dialog_new ();
  dialog_vbox = GTK_DIALOG (file_info_dialog)->vbox;
  action_area = GTK_DIALOG (file_info_dialog)->action_area;
  gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox), 5);
  gtk_signal_connect (GTK_OBJECT (file_info_dialog), "key_press_event",
		      GTK_SIGNAL_FUNC (key_press_cb), NULL);

  g_snprintf (label_text, sizeof (label_text), "Filename: %s",
	      info->filename);
  add_label (dialog_vbox, label_text, 0, TRUE, 5);

  add_separator (dialog_vbox, TRUE, 5);

  if (S_ISLNK (info->statbuf.st_mode))
    {
      gchar linkto[PATH_MAX + NAME_MAX];
      gchar *filename;
      int len;

      strncpy (label_text, "File Type: Symbolic Link", sizeof (label_text));
      add_label (dialog_vbox, label_text, 0, TRUE, 0);

      filename = strdup (info->filename);
      len = strlen (filename);
      if (filename[len - 1] == '/')
				filename[len - 1] = '\0';

      len = readlink (filename, linkto, sizeof (linkto));
      if (len > 0)
			{
	  		linkto[len] = '\0';
	  		g_snprintf (label_text, sizeof (label_text), "Link To: %s", linkto);
	  		add_label (dialog_vbox, label_text, 0, TRUE, 0);
			}
      else
			{
	  		strncpy (label_text, "Link To: Couldn't Resolve Link",
		   	sizeof (label_text));
	  		add_label (dialog_vbox, label_text, 0, TRUE, 0);
			}
      free (filename);
    }
  else if (S_ISREG (info->statbuf.st_mode))
    {
      gchar file_type[FILE_TYPE_LENGTH];
      gint len;

      get_file_type_using_file_command (info->filename, file_type,
					sizeof (file_type));
      if ((len = strlen (file_type)) > (FILE_TYPE_LENGTH - 2))
			{
	  		for (len = FILE_TYPE_LENGTH - 4; len < FILE_TYPE_LENGTH - 1; len++)
	    	{
	      	file_type[len] = '.';
	    	}
	  		file_type[FILE_TYPE_LENGTH - 1] = '\0';
			}
      else
			{
	  		file_type[len - 1] = '\0';
			}

      g_snprintf (label_text, sizeof (label_text), "File Type: %s",
		  file_type);
     
      add_label (dialog_vbox, label_text, 0, TRUE, 0);
    	}
  		else if (S_ISDIR (info->statbuf.st_mode))
    	{
      	strncpy (label_text, "File Type: Directory", sizeof (label_text));
      	add_label (dialog_vbox, label_text, 0, TRUE, 0);
    	}
  		else if (S_ISCHR (info->statbuf.st_mode))
    	{
      	strncpy (label_text, "File Type: Character Device",
	      sizeof (label_text));
      	add_label (dialog_vbox, label_text, 0, TRUE, 0);
    	}
  		else if (S_ISBLK (info->statbuf.st_mode))
    	{
      	strncpy (label_text, "File Type: Block Device", sizeof (label_text));
      	add_label (dialog_vbox, label_text, 0, TRUE, 0);
    	}
  		else if (S_ISFIFO (info->statbuf.st_mode))
    	{
      	strncpy (label_text, "File Type: FIFO Pipe", sizeof (label_text));
      	add_label (dialog_vbox, label_text, 0, TRUE, 0);
    	}
  	else if (S_ISSOCK (info->statbuf.st_mode))
    {
      strncpy (label_text, "File Type: Socket", sizeof (label_text));
      add_label (dialog_vbox, label_text, 0, TRUE, 0);
    }
  	else
    {
      strncpy (label_text, "File Type: Unknown", sizeof (label_text));
      add_label (dialog_vbox, label_text, 0, TRUE, 0);
    }

  	if (info->statbuf.st_size < 10240)	/* less than 10K */
   	 g_snprintf (label_text, sizeof (label_text),
			"Size: %d bytes", (int) info->statbuf.st_size);
  	else if (info->statbuf.st_size < 1048576)	/* less than a meg */
    	g_snprintf (label_text, sizeof (label_text),
			"Size: %.2f Kbytes", (float) info->statbuf.st_size / 1024.0);
  	else				/* more than a meg */
    	g_snprintf (label_text, sizeof (label_text),
			"Size: %.2f Mbytes",
		(float) info->statbuf.st_size / 1048576.0);

  	add_label (dialog_vbox, label_text, 0, TRUE, 0);

  	add_separator (dialog_vbox, TRUE, 5);

  	if ((pw_buf = getpwuid (info->statbuf.st_uid)) != NULL)
    {
      g_snprintf (label_text, sizeof (label_text),
		  "User: %s", pw_buf->pw_name);
    }
  	else
    {
      g_snprintf (label_text, sizeof (label_text),
		  "User: %d", (int) info->statbuf.st_uid);
    }
  	add_label (dialog_vbox, label_text, 0, TRUE, 0);

  	if ((grp_buf = getgrgid (info->statbuf.st_gid)) != NULL)
    {
      g_snprintf (label_text, sizeof (label_text),
		  "Group: %s", grp_buf->gr_name);
    }
  	else
    {
      g_snprintf (label_text, sizeof (label_text),
		  "Group: %d", (int) info->statbuf.st_gid);
    }
  	add_label (dialog_vbox, label_text, 0, TRUE, 0);

  	get_perm_string (perm_string, sizeof (label_text), info->statbuf.st_mode);
  	g_snprintf (label_text, sizeof (label_text), "Permissions: %s",
	      perm_string);
  	add_label (dialog_vbox, label_text, 0, TRUE, 0);

  	add_separator (dialog_vbox, TRUE, 5);

  	table = add_table (dialog_vbox, 3, 2, FALSE, TRUE, 5);
  	gtk_table_set_row_spacings (GTK_TABLE (table), 5);

  	tm_ptr = localtime (&info->statbuf.st_atime);
  	strftime (label_text, sizeof (label_text), "%a %b %d %I:%M %p", tm_ptr);
  	add_label_to_table (table, "Accessed: ", 0, 0, 1, 0, 1);
  	add_label_to_table (table, label_text, 0, 1, 2, 0, 1);

  	tm_ptr = localtime (&info->statbuf.st_mtime);
  	strftime (label_text, sizeof (label_text), "%a %b %d %I:%M %p", tm_ptr);
  	add_label_to_table (table, "Modified: ", 0, 0, 1, 1, 2);
  	add_label_to_table (table, label_text, 0, 1, 2, 1, 2);

  	tm_ptr = localtime (&info->statbuf.st_ctime);
  	strftime (label_text, sizeof (label_text), "%a %b %d %I:%M %p", tm_ptr);
  	add_label_to_table (table, "Changed: ", 0, 0, 1, 2, 3);
  	add_label_to_table (table, label_text, 0, 1, 2, 2, 3);

  	add_button (action_area, "Close", TRUE, 0, close_cb, file_info_dialog);

  	gtk_window_set_position (GTK_WINDOW (file_info_dialog), GTK_WIN_POS_CENTER);
  	gtk_widget_show (file_info_dialog);
  	gtk_grab_add (file_info_dialog);
}
