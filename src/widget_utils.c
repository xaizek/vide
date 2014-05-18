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

/* These all pretty much do the same thing.. create a widget and add it to
 * the box or table passed.. return the new widget
 */

void
add_tool_button (GtkWidget * toolbar, gchar * name, gchar * description,
		 GtkWidget * icon, GtkSignalFunc button_clicked)
{

  gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), name, description, "",
			   icon, GTK_SIGNAL_FUNC (button_clicked),
			   GTK_OBJECT (toolbar));
}

GtkWidget *
add_button (GtkWidget * box,
	    gchar * label,
	    gint fill, gint pad, GtkSignalFunc func, gpointer data)
{
  GtkWidget *button;
  button = gtk_button_new_with_label (label);
  gtk_signal_connect (GTK_OBJECT (button), "clicked", func, data);
  gtk_box_pack_start (GTK_BOX (box), button, fill, TRUE, pad);
  gtk_widget_show (button);

  return button;
}

GtkWidget *
add_label (GtkWidget * box, gchar * text, gfloat align, gint fill, gint pad)
{
  GtkWidget *label;
  label = gtk_label_new (text);
  gtk_misc_set_alignment (GTK_MISC (label), align, 0.5);
  gtk_box_pack_start (GTK_BOX (box), label, fill, TRUE, pad);
  gtk_widget_show(label);

  return label;
}

GtkWidget *
add_entry (GtkWidget * box, gchar * init_text, gint fill, gint pad)
{
  GtkWidget *entry;
  entry = gtk_entry_new ();
  if (init_text != NULL)
    gtk_entry_set_text (GTK_ENTRY (entry), init_text);
  gtk_box_pack_start (GTK_BOX (box), entry, fill, TRUE, pad);
  gtk_widget_show (entry);

  return entry;
}

GtkWidget *
add_check_button (GtkWidget * box,
		  gchar * label,
		  gint state,
		  gint fill, gint pad, GtkSignalFunc func, gpointer data)
{
  GtkWidget *check_button;
  check_button = gtk_check_button_new_with_label (label);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button), state);
  gtk_signal_connect (GTK_OBJECT (check_button), "toggled", func, data);
  gtk_box_pack_start (GTK_BOX (box), check_button, fill, TRUE, pad);
  gtk_widget_show (check_button);

  return check_button;
}

GtkWidget *
add_radio_button (GtkWidget * box,
		  gchar * label,
		  GSList * group,
		  gint fill, gint pad, GtkSignalFunc func, gpointer data)
{
  GtkWidget *radio_button;
  radio_button = gtk_radio_button_new_with_label (group, label);
  gtk_signal_connect (GTK_OBJECT (radio_button), "toggled", func, data);
  gtk_box_pack_start (GTK_BOX (box), radio_button, fill, TRUE, pad);
  gtk_widget_show (radio_button);

  return radio_button;
}

GtkWidget *
add_hbox (GtkWidget * box, gint homogen, gint spacing, gint fill, gint pad)
{
  GtkWidget *hbox;
  hbox = gtk_hbox_new (homogen, spacing);
  gtk_box_pack_start (GTK_BOX (box), hbox, fill, TRUE, pad);
  gtk_widget_show (hbox);

  return hbox;
}

GtkWidget *
add_vbox (GtkWidget * box, gint homogen, gint spacing, gint fill, gint pad)
{
  GtkWidget *vbox;
  vbox = gtk_vbox_new (homogen, spacing);
  gtk_box_pack_start (GTK_BOX (box), vbox, fill, TRUE, pad);
  gtk_widget_show (vbox);

  return vbox;
}

GtkWidget *
add_sw (GtkWidget * box,
	GtkPolicyType h_policy, GtkPolicyType v_policy, gint fill, gint pad)
{
  GtkWidget *sw;
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), h_policy,
				  v_policy);
  gtk_box_pack_start (GTK_BOX (box), sw, fill, TRUE, pad);
  gtk_widget_show (sw);

  return sw;
}

GtkWidget *
add_separator (GtkWidget * box, gint fill, gint pad)
{
  GtkWidget *sep;

  if (GTK_IS_HBOX (box))
    {
      sep = gtk_vseparator_new ();
      gtk_box_pack_start (GTK_BOX (box), sep, fill, TRUE, pad);
    }
  else if (GTK_IS_VBOX (box))
    {
      sep = gtk_hseparator_new ();
      gtk_box_pack_start (GTK_BOX (box), sep, fill, TRUE, pad);
    }
  else
    {
      return NULL;
    }

  gtk_widget_show (sep);
  return sep;
}

GtkWidget *
add_table (GtkWidget * box,
	   gint rows, gint cols, gint homogen, gint fill, gint pad)
{
  GtkWidget *table;
  table = gtk_table_new (rows, cols, homogen);
  gtk_box_pack_start (GTK_BOX (box), table, fill, TRUE, pad);
  gtk_widget_show (table);

  return table;
}

GtkWidget *
add_label_to_table (GtkWidget * table,
		    gchar * text,
		    gfloat align,
		    gint left, gint right, gint top, gint bottom)
{
  GtkWidget *label;
  label = gtk_label_new (text);
  gtk_misc_set_alignment (GTK_MISC (label), align, 0.5);
  gtk_table_attach_defaults (GTK_TABLE (table), label, left, right, top,
			     bottom);
  gtk_widget_show (label);

  return label;
}

GtkWidget *
add_entry_to_table (GtkWidget * table,
		    gchar * init_text,
		    gint left, gint right, gint top, gint bottom)
{
  GtkWidget *entry;
  entry = gtk_entry_new ();
  if (init_text != NULL)
    gtk_entry_set_text (GTK_ENTRY (entry), init_text);
  gtk_table_attach_defaults (GTK_TABLE (table), entry, left, right, top,
			     bottom);
  gtk_widget_show (entry);

  return entry;
}

GtkWidget *
add_button_to_table (GtkWidget * table,
		     gchar * label,
		     GtkSignalFunc func,
		     gpointer data,
		     gint left, gint right, gint top, gint bottom)
{
  GtkWidget *button;
  button = gtk_button_new_with_label (label);
  gtk_signal_connect (GTK_OBJECT (button), "clicked", func, data);
  gtk_table_attach_defaults (GTK_TABLE (table), button, left, right, top,
			     bottom);
  gtk_widget_show (button);

  return button;
}

GtkWidget *
add_check_button_to_table (GtkWidget * table,
			   gchar * label,
			   gint state,
			   GtkSignalFunc func,
			   gpointer data,
			   gint left, gint right, gint top, gint bottom)
{
  GtkWidget *check_button;
  check_button = gtk_check_button_new_with_label (label);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button), state);
  gtk_signal_connect (GTK_OBJECT (check_button), "toggled", func, data);
  gtk_table_attach_defaults (GTK_TABLE (table), check_button,
			     left, right, top, bottom);
  gtk_widget_show (check_button);

  return check_button;
}

GtkWidget *
add_menu_item (GtkWidget * menu,
	       gchar * label, GtkSignalFunc func, gpointer data)
{
  GtkWidget *menu_item;
  menu_item = gtk_menu_item_new_with_label (label);
  gtk_signal_connect (GTK_OBJECT (menu_item), "activate", func, data);
  gtk_menu_append (GTK_MENU (menu), menu_item);
  gtk_widget_show (menu_item);

  return menu_item;
}

GtkWidget *
add_menu_check_button (GtkWidget * menu,
		       gchar * label,
		       gboolean state, GtkSignalFunc func, gpointer data)
{
  GtkWidget *check;
  check = gtk_check_menu_item_new_with_label (label);
  gtk_signal_connect (GTK_OBJECT (check), "activate", func, data);
  gtk_menu_append (GTK_MENU (menu), check);
  gtk_check_menu_item_set_state (GTK_CHECK_MENU_ITEM (check), state);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (check), TRUE);
  gtk_widget_show (check);

  return check;
}

GtkWidget *
add_menu_separator (GtkWidget * menu)
{
  GtkWidget *menu_item;
  menu_item = gtk_menu_item_new ();
  gtk_menu_append (GTK_MENU (menu), menu_item);
  gtk_widget_show (menu_item);
  gtk_widget_set_sensitive (GTK_WIDGET (menu_item), FALSE);

  return menu_item;
}

GtkWidget *
add_submenu (GtkWidget * menu_bar, gchar * label_text, GtkWidget * menu)
{
  GtkWidget *menu_item;
  GtkWidget *label;

  menu_item = gtk_menu_item_new ();
  label = gtk_widget_new (GTK_TYPE_LABEL,
			  "GtkWidget::visible", TRUE,
			  "GtkWidget::parent", menu_item,
			  "GtkMisc::xalign", 0.0, NULL);
  gtk_label_parse_uline (GTK_LABEL (label), label_text);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), menu);
  gtk_menu_bar_append (GTK_MENU_BAR (menu_bar), menu_item);
  gtk_widget_show (menu_item);

  return menu_item;
}



GtkWidget *
create_pixmap (GtkWidget * widget, const gchar * file)
{

  GdkColormap *colormap;
  GdkPixmap *gdkpixmap;
  GdkBitmap *mask;
  GtkWidget *pixmap;
  GtkStyle *style;

  colormap = gtk_widget_get_colormap (app.main_window);
  style = gtk_widget_get_style (app.main_window);
  gdkpixmap = gdk_pixmap_colormap_create_from_xpm (NULL, colormap, &mask,
						   NULL, file);
  pixmap = gtk_pixmap_new (gdkpixmap, mask);
  gdk_pixmap_unref (gdkpixmap);
  gdk_bitmap_unref (mask);
  return pixmap;
}

/*
GtkWidget *
add_submenu(GtkWidget *menu_bar, gchar *label, GtkWidget *menu)
{
  GtkWidget *menu_item;
  menu_item = gtk_menu_item_new_with_label(label);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), menu);
  gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), menu_item);
  gtk_widget_show(menu_item);
 
  return menu_item;
}
*/
