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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include "vide.h"

/*
 * This file contains functions used to handle different file operations
 * like copying, deleting, sym linking, etc.. most functions do this by 
 * forking and running the standard unix commands for the appropriate operation
 */

static void start_reader_thread (void *ptr);

/* executes the argument array passed to it and waits for
 * the forked process to exit before returning. It is used to do most
 * of the file operations.
static gint
exec_and_wait(args)
  gchar **args;
{
  gint pid, status;

  pid = fork();
  if (pid == -1)
    return -1;
  if (pid == 0)
  {
    execvp(args[0], args);
    exit(127);
  }

  do
  {  
    if (waitpid(pid, &status, 0) == -1)
    {
      if (errno != EINTR)
        return -1;
    }
    else
      return status;
  } while(1);
}
 */

/* simply executes the argument array passed. the sigchild handler set up
 * during startup will handle the process when it has ended so that we dont
 * get zombies
 */
static gint
execute (gchar ** args)
{
  gint pid;

  if ((pid = fork ()) == 0)
    {
			close(0);
      execvp (args[0], args);
      exit (127);
    }

  return pid;
}

/* copy src to dest.. return when the operation is done */
gint file_copy (gchar * src, gchar * dest)
{
  gchar *args[5];

  args[0] = "cp";
  args[1] = "-r";
  args[2] = src;
  args[3] = dest;
  args[4] = NULL;

  return pipe_and_capture_output (args);
}

/* delete file.. return when the operation is done */
gint file_delete (gchar * file)
{
  gchar *args[4];

  args[0] = "rm";
  args[1] = "-rf";
  args[2] = file;
  args[3] = NULL;

  return pipe_and_capture_output (args);
}

/* move src to dest.. return when the operation is done */
gint file_move (gchar * src, gchar * dest)
{
  gchar *args[4];

  args[0] = "mv";
  args[1] = src;
  args[2] = dest;
  args[3] = NULL;

  return pipe_and_capture_output (args);
}

gint file_chmod (gchar * path, gchar * mode, gboolean recurse_dirs)
{
  gchar *args[5];
  gint i = 0;

  args[i++] = "chmod";
  if (recurse_dirs)
    args[i++] = "-R";
  args[i++] = mode;
  args[i++] = path;
  args[i++] = NULL;

  return pipe_and_capture_output (args);
}

gint
file_chown (gchar * path,
	    uid_t owner_id, gid_t group_id, gboolean recurse_dirs)
{
  gchar *args[5];
  gchar mode[14];
  gint i = 0;

  g_snprintf (mode, sizeof (mode), "%d:%d", (int) owner_id, (int) group_id);

  args[i++] = "chown";
  if (recurse_dirs)
    args[i++] = "-R";
  args[i++] = mode;
  args[i++] = path;
  args[i++] = NULL;

  return pipe_and_capture_output (args);
}

/* symlink src to dest */
gint file_symlink (gchar * src, gchar * dest)
{
  return symlink (src, dest);
}

gint file_mkdir (gchar * path)
{
  return mkdir (path, 0777);
}

/* execute command.. do not wait for it to exit */
gint file_exec (gchar * command)
{
  gchar *args[4];
  pid_t pid;

  args[0] = "sh";
  args[1] = "-c";
  args[2] = command;
  args[3] = NULL;

  pid = execute (args);
  return pid;
}

gint exec_in_xterm (gchar * command)
{
	/* gnome-terminal takes different aguments than xterm or rxvt */
	if(!strcmp(cfg.xterm_command, "gnome-terminal"))
	{
		gchar *args[4];
		gchar buf[PATH_MAX];

		g_snprintf(buf, sizeof(buf), "'%s'", command);

		args[0] = cfg.xterm_command;
		args[1] = "-e";
		args[2] = command;
		args[3] = NULL;

		return execute(args);

	}
	else
	{
  	gchar *args[7];

  	args[0] = cfg.xterm_command;
  	args[1] = "+sb";
  	args[2] = "-e";
  	args[3] = "sh";
  	args[4] = "-c";
  	args[5] = command;
  	args[6] = NULL;

		return execute(args);
	}

}

gint pipe_and_capture_output (gchar ** args)
{
  gint file_pipes[2];
  gint pid;
  gchar buf[1024];
  gint nread;

  if (pipe (file_pipes) != 0)
    {
      status_errno ();
      return -1;
    }

  if ((pid = fork ()) == -1)
    {
      status_errno ();
      return -1;
    }

  if (pid == 0)
    {
      close (1);
      close (2);
      dup (file_pipes[1]);
      dup (file_pipes[1]);
      close (file_pipes[0]);
      close (file_pipes[1]);

      execvp (args[0], args);
      exit (127);
    }
  else
    {
      close (file_pipes[1]);
      while ((nread = read (file_pipes[0], buf, sizeof (buf) - 1)) > 0)
	{
	  buf[nread] = '\0';
	     status_bar_message(buf);
	}
      close (file_pipes[0]);
    }

  return 0;
}

gint pipe_and_capture_output_threaded (gchar * command)
{
  gint file_pipes[2];
  gint pid;
  gchar *args[4];
  pthread_t reader_thread;


  if (pipe (file_pipes) != 0)
    {
      status_errno ();
      return -1;
    }

  if ((pid = fork ()) == -1)
    {
      status_errno ();
      return -1;
    }

  if (pid == 0)
    {
      close (1);
      close (2);
      dup (file_pipes[1]);
      dup (file_pipes[1]);
      close (file_pipes[0]);
      close (file_pipes[1]);

      args[0] = "sh";
      args[1] = "-c";
      args[2] = command;
      args[3] = NULL;
      execvp (args[0], args);
      exit (127);
    }
  else
    {
      close (file_pipes[1]);
      pthread_create (&reader_thread, NULL, (void *) &start_reader_thread,
		      duplicate (&file_pipes[0], sizeof (gint)));
    }

  return 1;
}

static void
start_reader_thread (void *ptr)
{
  gchar buf[1024];
  gint nread, error;
  gint *file_pipe = (gint *) ptr;
	error = 0;

  while ((nread = read (*file_pipe, buf, sizeof (buf) - 1)) > 0)
    {
      buf[nread] = '\0';
			error =  nread;
    }
	/* create error message dialog FIXME */
	/* not all errors are returned correctly */
	if(error != 0)
	{
			create_error_dialog(buf);
	}
  close (*file_pipe);
  g_free (file_pipe);
  pthread_exit (0);
}
