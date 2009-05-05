/*
* gedit-typingtutor-plugin.c
* Copyright (C) 2009  "Mohan Raman" <mohan43u@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>. 
*
*/

#include "gedit-typingtutor-plugin.h"

/* 
* Function to calculate elapsed time
*/

static void
print_elapsed_time(GtkWidget *timerlabel,
	GTimer *timer)
{
	char elapsedtimestring[20];
	time_t elapsedtime = (time_t) g_timer_elapsed(timer,NULL);
	struct tm elapsed;

	localtime_r(&elapsedtime,&elapsed);

	/*
	* 'struct tm' defaults tm_min to 30
	* and tm_hour to 5, Need to subtract
	* the default values to get what we
	* want.
	*/

	elapsed.tm_min = elapsed.tm_min - 30;
	elapsed.tm_hour = elapsed.tm_hour - 5;

	strftime(elapsedtimestring,
		20,
		"%T",
		&elapsed);

	gtk_label_set_text(GTK_LABEL(timerlabel),
		g_strdup_printf("Elapsed Time: %s",
			elapsedtimestring));
}

/*
* Function to start and stop GTimer
*/

static void
toggletimer_callback(GtkToggleButton *toggle,
	gpointer user_data)
{
	GeditTypingTutorPlugin *args = (GeditTypingTutorPlugin *) user_data;

	if(!g_strcmp0("_Start Timer",gtk_button_get_label(GTK_BUTTON(toggle))))
	{
		g_timer_start(args->timer);
		gtk_button_set_label(GTK_BUTTON(toggle), "_Stop Timer");
	}
	else
	{
		g_timer_stop(args->timer);
		print_elapsed_time(args->timerlabel, args->timer);
		gtk_button_set_label(GTK_BUTTON(toggle), "_Start Timer");
	}
}

/*
* Callback function for GeditDocument 'insert-text'
* signal. This function will check whether the
* inserted character is same or not. Also updates
* elapsed time for every keystroke.
*/

static void
document_callback(GtkTextBuffer *textbuffer,
	GtkTextIter *location,
	gchar *text,
	gint len,
	gpointer user_data)
{
	GeditTypingTutorPlugin *args = (GeditTypingTutorPlugin *) user_data;
	GeditDocument *document = args->doc;
	GtkTextIter start,end;
	GtkTextIter start_new;
	gchar *text1;
	glong length = 0;

	/*
	* Calculating 'utf8' length of
	* inserted character.
	*/

	length = g_utf8_strlen(text,len);

	gtk_text_buffer_get_iter_at_offset(
		GTK_TEXT_BUFFER(document),
		&start,
		(gtk_text_iter_get_offset(location) - length));

	gtk_text_buffer_get_iter_at_offset(
		GTK_TEXT_BUFFER(document),
		&end,
		gtk_text_iter_get_offset(location));

	text1 = gtk_text_buffer_get_text(
		GTK_TEXT_BUFFER(document),
		&start,
		&end,
		FALSE);

	gtk_text_buffer_get_iter_at_offset(textbuffer,
		&start_new,
		(gtk_text_iter_get_offset(location) - length));

	/*
	* For debugging porpose
	*
	g_print("Text1 Start: %d\n"
		"Text1 End: %d\n"
		"Text Start: %d\n"
		"Text End: %d\n"
		"Byte Length: %d\n"
		"Unicode Length: %d\n"
		"Text1: %s\n"
		"Text: %s\n",
		gtk_text_iter_get_offset(&start),
		gtk_text_iter_get_offset(&end),
		gtk_text_iter_get_offset(&start_new),
		gtk_text_iter_get_offset(location),
		len,
		length,
		text1,
		text);
	*/
		
	if(g_strcmp0(text1, text))
	{
		gtk_text_buffer_apply_tag(textbuffer,
			args->tag,
			&start_new,
			location);

		args->mistakecount = args->mistakecount + 1;
		gtk_label_set_text(GTK_LABEL(args->mistakelabel),
			g_strdup_printf("Mistake Count(keys): %ld",
				args->mistakecount));
	}

	print_elapsed_time(args->timerlabel, args->timer);

	args->keystrokecount = args->keystrokecount + 1;
	gtk_label_set_text(GTK_LABEL(args->keystrokelabel),
		g_strdup_printf("Keystroke Count(keys): %ld",
			args->keystrokecount));
}

/*
* plugin callback for 'activate' signal from GtkAction, this
* function will take care of the comparison and calculation
* of 'TypingTutor' plugin.
*/

static void
gedit_typingtutor_plugin_callback(GtkAction *action,gpointer data)
{
	GeditWindow *window;
	GeditTab *tab;
	GtkWidget *view,*view_new;
	GeditDocument *document,*document_new;
	GtkWidget *win;
	GtkWidget *vbox;
	GtkAdjustment *hadjustment, *vadjustment;
	GtkWidget *scrolledwindow1, *scrolledwindow2;
	GtkTextTag *mistake;
	GeditTypingTutorPlugin *args = g_new0(GeditTypingTutorPlugin, 1);
	GtkWidget *hbox;
	GtkWidget *mistakelabel,*keystrokelabel, *timerlabel;
	GtkWidget *toggletimer;
	GTimer *timer;
	GtkWidget *messagebox;

	window = GEDIT_WINDOW(data);
	tab = gedit_window_get_active_tab(window);
	document = gedit_tab_get_document(tab);
	view = gedit_view_new(document);

	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(win),
		"delete-event",
		G_CALLBACK(gtk_widget_destroy),
		NULL);

	if(!gtk_text_buffer_get_char_count(GTK_TEXT_BUFFER(document)))
	{
		messagebox = gtk_message_dialog_new(GTK_WINDOW(win),
			GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_OK,
			"No File\n\n"
			"open any file in gedit and click Tools->TypingTutor");
		g_signal_connect_swapped(G_OBJECT(messagebox),
			"response",
			G_CALLBACK(gtk_widget_destroy),
			(gpointer) win);
		gtk_widget_show(messagebox);
	}

	document_new = gedit_document_new();
	view_new = gedit_view_new(document_new);
	mistake = gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(document_new),
		"mistake",
		"background", "red",
		NULL);

	vbox = gtk_vbox_new(FALSE, 10);
	hbox = gtk_hbox_new(FALSE, 10);
	toggletimer = gtk_toggle_button_new_with_mnemonic("_Start Timer");
	mistakelabel = gtk_label_new("Mistake Count(keys): 0");
	timerlabel = gtk_label_new("Elapsed time:");
	keystrokelabel = gtk_label_new("Keystroke Count(keys): 0");
	timer = g_timer_new();

	args->doc = document;
	args->tag = mistake;
	args->mistakelabel = mistakelabel;
	args->keystrokelabel = keystrokelabel;
	args->timerlabel = timerlabel;
	args->timer = timer;

	g_signal_connect(G_OBJECT(toggletimer),
		"toggled",
		G_CALLBACK(toggletimer_callback),
		(gpointer) args);

	g_signal_connect_after(G_OBJECT(document_new),
		"insert-text",
		G_CALLBACK(document_callback),
		(gpointer) args);

	hadjustment = gtk_adjustment_new(1.0,
		1.0,
		10.0,
		1.0,
		3.0,
		10.0);

	vadjustment = gtk_adjustment_new(1.0,
		1.0,
		10.0,
		1.0,
		3.0,
		10.0);

	scrolledwindow1 = gtk_scrolled_window_new(hadjustment,
		vadjustment);

	scrolledwindow2 = gtk_scrolled_window_new(hadjustment,
		vadjustment);

	gtk_container_add(GTK_CONTAINER(scrolledwindow1),
		view);

	gtk_container_add(GTK_CONTAINER(scrolledwindow2),
		view_new);

	gtk_box_pack_start(GTK_BOX(hbox),
		toggletimer,
		TRUE,
		TRUE,
		10);

	gtk_box_pack_start(GTK_BOX(hbox),
		timerlabel,
		TRUE,
		TRUE,
		10);

	gtk_box_pack_start(GTK_BOX(hbox),
		keystrokelabel,
		TRUE,
		TRUE,
		10);

	gtk_box_pack_start(GTK_BOX(hbox),
		mistakelabel,
		TRUE,
		TRUE,
		10);

	gtk_box_pack_start(GTK_BOX(vbox),
		scrolledwindow1,
		TRUE,
		TRUE,
		10);

	gtk_box_pack_start(GTK_BOX(vbox),
		hbox,
		TRUE,
		TRUE,
		10);

	gtk_box_pack_start(GTK_BOX(vbox),
		scrolledwindow2,
		TRUE,
		TRUE,
		10);

	gtk_container_add(GTK_CONTAINER(win), vbox);
	gtk_window_set_default_size(GTK_WINDOW(win),
		800,
		500);
	gtk_widget_set_size_request(scrolledwindow1,
		800,
		200);
	gtk_widget_set_size_request(scrolledwindow2,
		750,
		200);
	gtk_window_set_title(GTK_WINDOW(win),
		"TypingTutor");
	gtk_window_set_focus(GTK_WINDOW(win),
		toggletimer);
	gtk_widget_show_all(win);
}

/*
* Plugin Register call. This will register
* GeditTypingTutorPlugin into runtime GObject system.
*/

GEDIT_PLUGIN_REGISTER_TYPE(GeditTypingTutorPlugin, \
	gedit_typingtutor_plugin)

static void
gedit_typingtutor_plugin_init(GeditTypingTutorPlugin *plugin)
{
}

static void
gedit_typingtutor_plugin_finalize(GeditTypingTutorPlugin *plugin)
{
}

static guint typingtutor_merge_id = 0;

/*
* Deactivate function to remove 'TypingTutor' from Tools menu
*/

static void
impl_deactivate(GeditPlugin *plugin, GeditWindow *window)
{
	GtkUIManager *ui_manager;
	GtkActionGroup *action_group;
	GtkAction *action;
	GList *list;

	ui_manager = gedit_window_get_ui_manager(window);
	list = gtk_ui_manager_get_action_groups(ui_manager);

	while(list)
	{
		action_group = GTK_ACTION_GROUP(list->data);
		if(!g_strcmp0("GeditWindowActions",
			gtk_action_group_get_name(action_group)))
		{
			action = gtk_action_group_get_action(action_group,
				"TypingTutor");
			gtk_action_group_remove_action(action_group,
				action);
			gtk_ui_manager_remove_ui(ui_manager,
				typingtutor_merge_id);
			break;
		}
		list = g_list_next(list);
	}
}

/*
* Activate function to add 'TypingTutor' in 'Tools' menu.
*/

static void
impl_activate(GeditPlugin *plugin, GeditWindow *window)
{
	GtkUIManager *ui_manager;
	GtkActionGroup *action_group;
	GtkAction *action;
	GList *list;

	action = gtk_action_new("TypingTutor",
		"_TypingTutor",
		"compares text",
		NULL);

	g_signal_connect(G_OBJECT(action),
		"activate",
		G_CALLBACK(gedit_typingtutor_plugin_callback),
		(gpointer) window);

	ui_manager = gedit_window_get_ui_manager(window);
	list = gtk_ui_manager_get_action_groups(ui_manager);

	while(list)
	{
		action_group = GTK_ACTION_GROUP(list->data);
		if(!g_strcmp0("GeditWindowActions",
			gtk_action_group_get_name(action_group)))
		{
			gtk_action_group_add_action(action_group, action);
			typingtutor_merge_id = gtk_ui_manager_new_merge_id(
				ui_manager);
			gtk_ui_manager_add_ui(ui_manager,
				typingtutor_merge_id,
				"/MenuBar/ToolsMenu",
				"TypingTutor",
				"TypingTutor",
				GTK_UI_MANAGER_MENUITEM,
				TRUE);
			break;
		}
		list = g_list_next(list);
	}
}
				
static void
impl_update_ui(GeditPlugin *plugin, GeditWindow *window)
{
}

/*
* Plugin Class init function
*/

static void
gedit_typingtutor_plugin_class_init(GeditTypingTutorPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GeditPluginClass *plugin_class = GEDIT_PLUGIN_CLASS(klass);

	object_class->finalize = gedit_typingtutor_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
	plugin_class->update_ui = impl_update_ui;
}
