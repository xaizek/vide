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
#include "vide.h"
void
add_user_command (gchar * name, gchar * action)
{
  UserCommand command;

  strncpy (command.name, name, sizeof (command.name));
  strncpy (command.action, action, sizeof (command.action));
  cfg.user_commands = g_list_append (cfg.user_commands,
				     duplicate (&command,
						sizeof (UserCommand)));
}
