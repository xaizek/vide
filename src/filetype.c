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

/* Allocates and returns the programs string in the file types list with a
 * matching extension.. the value returned needs to be freed
 */
gchar *
get_programs_for_ext (gchar * ext)
{
  FileType *ft = get_filetype_for_ext (ext);

  if (ft != NULL)
    return strdup (ft->programs);

  return NULL;
}

/* same as above but returns the first program in the list */
gchar *
get_default_program_for_ext (gchar * ext)
{
  FileType *ft = get_filetype_for_ext (ext);

  if (ft != NULL)
    {
      gchar *program = strdup (ft->programs);
      gchar *s;

      if ((s = strchr (program, ',')) != NULL)
	*s = '\0';

      return program;
    }

  return NULL;
}

FileType *
get_filetype_for_ext (gchar * ext)
{
  GList *tmp;
  gchar *ext_copy, *s, *s2, *free_this;

  ext = str_to_lower (ext);
  for (tmp = cfg.filetypes; tmp != NULL; tmp = tmp->next)
    {
      FileType *ft = tmp->data;

      free_this = ext_copy = strdup (ft->extensions);
      while ((s = s2 = strchr (ext_copy, ',')) != NULL)
	{
	  while (*(s2 - 1) == ' ')	/* get rid of whitespace */
	    s2--;
	  *s2 = '\0';
	  while ((*ext_copy == '.') || (*ext_copy == ' '))
	    ext_copy++;

	  if (STREQ (ext_copy, ext))
	    {
	      free (free_this);
	      free (ext);
	      return ft;
	    }
	  ext_copy = s + 1;
	}
      while ((*ext_copy == '.') || (*ext_copy == ' '))
	ext_copy++;
      if (STREQ (ext_copy, ext))
	{
	  free (free_this);
	  free (ext);
	  return ft;
	}
      free (free_this);
    }
	g_list_free(tmp);
  free (ext);

  return NULL;
}

/* first checks to see if a filetype with the same extensions exists..
 * if not it creates and adds a new filetype
 */
void
add_filetype (gchar * extensions, gchar * programs, gchar * description)
{
  GList *tmp;

  for (tmp = cfg.filetypes; tmp != NULL; tmp = tmp->next)
    {
      FileType *ft = tmp->data;

      if (STREQ (extensions, ft->extensions))
	{
	  strncpy (ft->programs, programs, sizeof (ft->programs));
	  strncpy (ft->description, description, sizeof (ft->description));
	  return;
	}
    }

  {
    FileType ft;
    strncpy (ft.description, description, sizeof (ft.description));
    strncpy (ft.extensions, extensions, sizeof (ft.extensions));
    strncpy (ft.programs, programs, sizeof (ft.programs));

    cfg.filetypes =
      g_list_append (cfg.filetypes, duplicate (&ft, sizeof (FileType)));
  }
}

