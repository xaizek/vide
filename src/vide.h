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


#ifndef __VIDE_H__
#define __VIDE_H__

#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
/* #include <zvt/zvtterm.h> */

#define VERSION "0.5.2"
#define NAME_MAX 255
#define MAX_LEN 1024
#define MAX_BUTTONS 16
#define MAX_COLUMNS 8

#define STREQ(a,b) (strcmp((a),(b)) == 0)



typedef struct History{
	gchar dir[PATH_MAX];
	gchar file[PATH_MAX];
}History;

typedef struct Pattern{
	gchar pattern[PATH_MAX];
	int index;
}Pattern;

typedef struct Command{
	gchar buffer[1024];
	gint index;
	gchar number[100];
	gchar count[100];
}Command;

typedef enum
{ FILENAME = 0, SIZE, MODIFIED, ACCESSED, CHANGED,
  PERM, OWNER, GROUP
}
ColumnFlags;
typedef enum
{ GENERAL_1 = 0, GENERAL_2, BOOKMARKS, FILETYPES, BUTTONS,
  COLUMNS, COLORS
}
ConfigDialogPages;
typedef enum
{ GT, LT, EQ }
Operator;
typedef enum
{ YES, YES_TO_ALL, NO, CANCEL }
ConfirmOverwriteButtons;

typedef struct _FileInfo
{
  gchar filename[NAME_MAX];
  struct stat statbuf;
}
FileInfo;

typedef struct _FileType
{
  gchar description[NAME_MAX];
  gchar extensions[NAME_MAX];
  gchar programs[NAME_MAX];
}
FileType;

typedef struct _Button
{
  gchar *title;
  gint id;
  GtkSignalFunc callback;
}
Button;

typedef struct _Column
{
  gchar *title;
  gint size;
  gint id;
  gint is_visible;
  GtkCListCompareFunc sort_func;
}
Column;

typedef struct _UserCommand
{
  gchar name[NAME_MAX];
  gchar action[NAME_MAX];
}
UserCommand;

typedef struct _Command_Mode_Programs
{
	gchar name[NAME_MAX];
	gchar action[NAME_MAX];
}
Command_Mode_Programs;


typedef struct _Bookmarks
{
	gchar mark[5];
	gchar dir[PATH_MAX];
	gchar file[PATH_MAX];
}
Bookmarks;


typedef struct _TmpMarks
{
	gchar mark[5];
	gchar dir[PATH_MAX];
	gchar file[PATH_MAX];
}
TmpMarks;

typedef struct _Programs
{
	gchar name[NAME_MAX];
	gchar action[NAME_MAX];
}
Programs;

typedef struct _FileView
{
  GtkWidget *clist;
  GtkWidget *sort_arrows[MAX_COLUMNS];
  GtkWidget *filter_menu_item;	
	GtkWidget *dir_label;
  GList *tagged;
  GList *iter;
  GList *iter_base;
  GList *dir_history;
	GList *history;
	GList *menu_history;
  gchar dir[PATH_MAX];
	gchar glob[PATH_MAX];

  gboolean filter_directories;
  struct
  {
    gchar pattern[NAME_MAX];
    gboolean invert_mask;
    gboolean active;
  }
  filename_filter;
  struct
  {
    off_t size;
    Operator op;
    gboolean active;
  }
  size_filter;
  struct
  {
    time_t time;
    Operator op;
    enum
    { MTIME, ATIME, CTIME }
		time_type;
    gboolean active;
  }
  date_filter;
  time_t dir_mtime;
	time_t dir_ctime;
  gint row;
	gchar last_dir[PATH_MAX];
  gint last_row;
}
FileView;

typedef struct _App
{
  FileView left_view;
  FileView right_view;
  FileView command_view;
	FileView fileselector;
	FileView filetypes_view;
  GtkWidget *sw;
  GtkWidget *main_window;
  GtkWidget *main_window_vbox;
  GtkWidget *main_menu_bar;
	GtkWidget *notebook;
	GtkWidget *notebook_label;
  GtkWidget *sep;
  GtkWidget *label;
  GtkWidget *bookmark_menu;
  GtkWidget *bookmark_menu_item;
  GtkWidget *vide_menu_item;
  GtkWidget *options_menu_item;
	GtkWidget *tools_menu_item;
  GtkWidget *help_menu_item;
	GtkWidget *command_menu_item;
  GtkWidget *left_view_box;
  GtkWidget *right_view_box;
  GtkWidget *hbox;
  GtkWidget *status_bar;
	GtkWidget *mode_bar;
	GtkWidget *message_box;
  GtkWidget *filetypes_popup;
	GtkWidget *filetypes_clist;
	GtkWidget *term;
  GtkWidget *filetype_popup;
	GtkWidget *programs_menu;
	GtkWidget *history_menu;
	GtkWidget *mark_menu;
	GtkWidget *command_mode_menu;
	GtkWidget *delc_menu;
	GtkWidget *up_button;
	GtkWidget *buffer_menu;
	GtkWidget *split_menu;
	GtkWidget *vsplit_menu;
}
App;

typedef struct _Zvt
{
	gboolean terminal_bell;
	gboolean use_blinking_cursor;
	gboolean scroll_on_keystroke;
	gboolean scroll_on_output;
	gboolean swap_backspace;
	gboolean transparent;
	gboolean shaded;
	gboolean show_tabs;
	gboolean use_pixmap;
	gchar pixmap[PATH_MAX];
	gchar term_font[512];
	gint scrollbar_position;
	GtkPositionType tab_position;
	gchar term_color[24];
	gchar background[24];
	gfloat scroll_lines;
	GdkColor palette[18];
	GdkColor foreground[2];
}Zvt;


typedef struct _ZvtTerm
{
}ZvtTerm;

typedef struct _Config
{
  gchar viewer_command[NAME_MAX];
  gchar xterm_command[NAME_MAX];
  gchar left_startup_dir[PATH_MAX];
  gchar right_startup_dir[PATH_MAX];
  gchar config_dir[PATH_MAX];
  gchar trash[PATH_MAX];
	gchar start_filter[NAME_MAX];
  time_t config_mtime;
  time_t filetypes_mtime;
  time_t bookmarks_mtime;
  time_t user_commands_mtime;
	time_t programs_mtime;
	time_t command_mode_programs_mtime;
	time_t term_mtime;
  gboolean show_hidden;
	gboolean frozen;
  gboolean start_with_cwd; 
	gboolean start_with_curr_dirs;
	gboolean use_scrollbars;
	gboolean invert_filter;
	gboolean save_position;
	gboolean fileselector;
  gint window_width;
  gint window_height;
  gint window_xpos;
  gint window_ypos;
  gint dir_history_max_length;
  gint command_history_max_length;
  GList *filetypes;
  GList *bookmarks;
	GList *tmp_marks;
  GList *user_commands;
	GList *programs;
	GList *terms;
	GList *command_mode_programs;
	ZvtTerm *current_term;
	gint term_count;
	gchar vi_clone[24];
	gchar vi_command[48];
	gchar man_program[128];
}
Config;


extern App app;
extern Config cfg;
extern FileView *curr_view;
extern FileView *other_view;
extern FileView *command_view;
extern FileView *fileselector;
extern FileView *filetypes_view;
extern Bookmarks bk;
extern TmpMarks tmark;
extern Pattern pat;
extern Command com;
extern History history;
extern Zvt zvt;


extern Column all_columns[MAX_COLUMNS];

/* Colors */
extern GdkColor EXE_COLOR;
extern GdkColor DIR_COLOR;
extern GdkColor LNK_COLOR;
extern GdkColor DEV_COLOR;
extern GdkColor COL_COLOR;
extern GdkColor TAG_COLOR;
extern GdkColor SOCK_COLOR;
extern GdkColor CLIST_COLOR;
extern GdkColor DRAG_HILIGHT;
extern GdkColor TAB_COLOR;


/* Widget Utils */
extern GtkWidget *add_button (GtkWidget * box, gchar * label, gint fill,
			      gint pad, GtkSignalFunc func, gpointer data);
extern GtkWidget *add_label (GtkWidget * box, gchar * text, gfloat align,
			     gint fill, gint pad);
extern GtkWidget *add_entry (GtkWidget * box, gchar * init_text, gint fill,
			     gint pad);
extern GtkWidget *add_check_button (GtkWidget * box, gchar * label,
				    gint state, gint fill, gint pad,
				    GtkSignalFunc func, gpointer data);
extern GtkWidget *add_radio_button (GtkWidget * box, gchar * label,
				    GSList * group, gint fill, gint pad,
				    GtkSignalFunc func, gpointer data);
extern GtkWidget *add_hbox (GtkWidget * box, gint homogen, gint spacing,
			    gint fill, gint pad);
extern GtkWidget *add_vbox (GtkWidget * box, gint homogen, gint spacing,
			    gint fill, gint pad);
extern GtkWidget *add_sw (GtkWidget * box, GtkPolicyType h_policy,
			  GtkPolicyType v_policy, gint fill, gint pad);
extern GtkWidget *add_separator (GtkWidget * box, gint fill, gint pad);
extern GtkWidget *add_table (GtkWidget * box, gint rows, gint cols,
			     gint homogen, gint fill, gint pad);
extern GtkWidget *add_label_to_table (GtkWidget * table, gchar * text,
				      gfloat align, gint left, gint right,
				      gint top, gint bottom);
extern GtkWidget *add_entry_to_table (GtkWidget * table, gchar * init_text,
				      gint left, gint right, gint top,
				      gint bottom);
extern GtkWidget *add_button_to_table (GtkWidget * table, gchar * label,
				       GtkSignalFunc func, gpointer data,
				       gint left, gint right, gint top,
				       gint bottom);
extern GtkWidget *add_check_button_to_table (GtkWidget * table, gchar * label,
					     gint state, GtkSignalFunc func,
					     gpointer data, gint left,
					     gint right, gint top,
					     gint bottom);
extern GtkWidget *add_menu_item (GtkWidget * menu, gchar * label,
				 GtkSignalFunc func, gpointer data);
extern GtkWidget *add_menu_check_button (GtkWidget * menu, gchar * label,
					 gboolean state, GtkSignalFunc func,
					 gpointer data);
extern GtkWidget *add_menu_separator (GtkWidget * menu);
extern GtkWidget *add_submenu (GtkWidget * menu_bar, gchar * label,
			       GtkWidget * menu);

/* Utils */
extern void *duplicate (void *stuff, gint size);
extern gchar *str_to_lower (gchar * string);
extern void set_cursor_watch ();
extern void set_cursor_normal ();
extern gint S_ISEXE (mode_t mode);
extern void free_clist_row_data (gpointer data);
extern void my_glist_free (GList ** list);
extern GList *string_glist_find (GList * list, gchar * search_text);
extern void untag_all (FileView * view);
extern GList *get_selection (FileView * view);
extern GList *clist_get_selected_row_data (GtkWidget * clist);
extern gint is_text (gchar * filename);
extern void chomp (gchar * text);
extern GString *expand_macros (gchar * text);
extern GString *list_macros (gchar * text);
extern void describe_file_size (gchar* buf, int buf_size, FileInfo* file_info);
extern void describe_user_id (gchar* buf, int buf_size, FileInfo* file_info);
extern void describe_group_id (gchar* buf, int buf_size, FileInfo* file_info);
extern void show_file_properties(void);
extern void show_full_file_properties(void);
 
/* Filetypes */
extern gchar *get_programs_for_ext (gchar * ext);
extern gchar *get_default_program_for_ext (gchar * ext);
extern FileType *get_filetype_for_ext (gchar * ext);
extern void add_filetype (gchar * ext, gchar * prog, gchar * desc);
extern void create_filetype_command_clist();

/* User Commands */
extern void add_user_command (gchar * name, gchar * action);
extern void load_user_commands(void);
extern void file_list_key_press_cb(GtkWidget *widget, GdkEventKey *event,
		FileView *view);
extern void fileselector_key_cb(GtkWidget *widget, GdkEventKey *event,
		gpointer data);
extern void command_select_row_cb(GtkWidget *clist, gint row, gint col,
			GdkEvent *event, FileView *view);
extern void program_select_row_cb(GtkWidget *clist, gint row, gint col,
			GdkEvent *event, FileView *view);

/* Programs */
extern void add_program(gchar *name, gchar *action);
extern void create_programs_menu(void);

/* Command Mode Programs */
extern void add_command(gchar *name, gchar *action);
extern void execute_command(gchar *action);

/* Callbacks */
extern void permissions_cb (GtkWidget * widget);
extern void ownership_cb (GtkWidget * widget);
extern void file_info_cb (GtkWidget * widget);
extern void sync_dirs_cb (GtkWidget * widget);
extern void mkdir_cb (GtkWidget * widget);
extern void toggle_tag_cb ();
extern void command_cb (GtkWidget * widget);
extern void configure_cb (GtkWidget * widget);
extern void quit_cb (GtkWidget * widget);
extern void toggle_tag_cb();
extern void find_next(FileView *view);
extern void find_prev(FileView *view);
extern void find_pattern(FileView *view);

/* Window */
extern void status_bar_message (gchar * msg);
extern void status_message (gchar * msg);
extern void status_errno ();
extern void create_main_window ();
extern void recreate_main_window ();
extern void search_mode(FileView *view);
extern void command_mode(FileView *view);

/* Term */
extern gint create_zterm(gchar *command, gboolean new_page);
extern void write_to_term(ZvtTerm *term, gchar *message);
extern void write_to_all_vi_terms(gchar *message);
extern ZvtTerm * get_nth_zvt(GtkNotebook *notebook, gint n);
extern ZvtTerm * get_focus_zvt(GtkNotebook *notebook, gint page_number);
extern void update_all_terms();
extern GtkWidget *create_term_menu(ZvtTerm *term, gchar *command);
extern void create_fileselector(GtkWidget *widget, gpointer data);

/* Filelist */
extern void get_perm_string (gchar * string, gint len, mode_t mode);
extern gint is_dir (FileInfo * info);
extern void change_dir (FileView * view, gchar * path);
extern gint name_sort (GtkCList * clist, gconstpointer row1,
		       gconstpointer row2);
extern gint size_sort (GtkCList * clist, gconstpointer row1,
		       gconstpointer row2);
extern gint date_sort (GtkCList * clist, gconstpointer row1,
		       gconstpointer row2);
extern gint perm_sort (GtkCList * clist, gconstpointer row1,
		       gconstpointer row2);
extern gint user_sort (GtkCList * clist, gconstpointer row1,
		       gconstpointer row2);
extern gint group_sort (GtkCList * clist, gconstpointer row1,
			gconstpointer row2);
extern void sort_list (FileView * view, GtkCListCompareFunc,
		       GtkSortType direction, gint col);
extern void focus_on_row (FileView * view, gint row);
extern void goto_row (GtkWidget * clist, gint row);
extern void initialize_filters (FileView * view);
extern void set_filter_menu_active (FileView * view);
extern void remove_filters (FileView * view);
extern void refresh_list (FileView * view);
extern void load_dir_list (FileView * view);
extern void exec_action (gchar * action);
extern void handle_file (FileView * view);

/*Bookmarks */
extern void add_bookmark(gchar *mark, gchar *dir, gchar *file);
extern void add_tmp_mark(gchar *mark, gchar *dir, gchar *file);

/* Menu */
extern GtkWidget *create_main_menu_bar ();
extern void create_user_popup ();
extern void create_special_popup_menu ();
extern void create_user_command_menu ();
extern void show_popup_menu (guint button, guint32 time);
extern void show_special_popup_menu (guint button, guint32 time);
extern void show_user_command_menu (guint button, guint32 time);
extern void load_bookmarks ();
extern void goto_mark(FileView *view);
extern void set_mark(FileView *view);
extern void set_menu_position(GtkMenu *menu, gint *x, gint *y, gpointer data);
extern void create_history_menu(FileView *view);
extern void bookmark_key_cb(GtkWidget *widget, GdkEventKey *event, FileView *view);
extern void create_command_mode_menu(void);
extern void create_delc_menu(void);
extern void create_buffer_menu(void);

/* File Ops */
extern gint file_copy (gchar * src, gchar * dest);
extern gint file_move (gchar * src, gchar * dest);
extern gint file_delete (gchar * file);
extern gint file_chmod (gchar * path, gchar * mode, gboolean recurse_dirs);
extern gint file_chown (gchar * path, uid_t owner_id, gid_t group_id,
			gboolean recurse_dirs);
extern gint file_mkdir (gchar * path);
extern gint file_symlink (gchar * src, gchar * dest);
extern gint file_exec (gchar * command);
extern gint exec_in_xterm (gchar * command);
extern gint pipe_and_capture_output (gchar ** args);
extern gint pipe_and_capture_output_threaded (gchar * command);
extern void block_signal(void);
extern void unblock_signal(void);

/* Dialogs */
extern void create_copy_as_dialog ();
extern void create_rename_dialog ();
extern void create_mkdir_dialog ();
extern void create_symlink_as_dialog ();
extern void create_permissions_dialog (FileInfo * info);
extern void create_ownership_dialog (FileInfo * info);
extern void create_confirm_del_dialog (gchar * filename, guint * answer);
extern void create_confirm_overwrite_dialog (gchar * filename,
					     guint * answer);
extern void create_command_dialog ();
extern void create_filetype_dialog (FileType * ft, gboolean write_file);
extern void create_init_filetype_dialog (gchar * ext, GtkWidget * clist);
extern void create_open_with_dialog ();
extern void create_config_dialog ();
extern void create_add_ext_dialog ();
extern void create_file_info_dialog ();
extern void create_view_file_dialog (gchar * filename);
extern void create_filename_filter_dialog (FileView * view);
extern void create_size_filter_dialog (FileView * view);
extern void create_date_filter_dialog (FileView * view);
extern void create_glob_dialog ();
extern void create_copy_here_dialog ();
extern void create_rename_ext_dialog ();
extern void create_user_prompt (gchar * prompt, gchar * init_text,
				gchar ** string);
extern void create_user_command_dialog (UserCommand ** command);
extern void create_programs_dialog(Programs **program);
extern void create_confirm_dialog (gchar * label_text, guint * answer);
extern void create_for_each_dialog ();
extern void create_user_popup ();
extern void create_filetype_popup(void);
extern void create_error_dialog(gchar * label_text);


/* Config Functions */
extern void set_config_dir ();
extern void write_filetypes_file ();
extern void write_bookmarks_file ();
extern void write_user_commands_file ();
extern void write_config_file ();
extern void write_programs_file();
extern void write_command_mode_file();
extern void write_term_file();
extern gboolean read_filetypes_file ();
extern gboolean read_bookmarks_file ();
extern gboolean read_user_commands_file ();
extern gboolean read_config_file ();
extern gboolean read_programs_file();
extern gboolean read_command_mode_file();
extern gboolean read_term_file();

#endif /* __VIDE_H__ */
