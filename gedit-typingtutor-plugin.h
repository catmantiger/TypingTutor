/*
* gedit-typingtutor-plugin.h
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
* along with this program.  If not, see .
*
*/

#ifndef __GEDIT_TYPINGTUTOR_PLUGIN_H__
#define __GEDIT_TYPINGTUTOR_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <gedit/gedit-plugin.h>
#include <gedit/gedit-app.h>
#include <gedit/gedit-window.h>

G_BEGIN_DECLS

/*
 * Instance
 */

typedef struct _GeditTypingTutorPlugin
{
	GeditPlugin parent_instance;
	GeditDocument *doc;
	GtkTextTag *tag;
	glong mistakecount;
	glong keystrokecount;
	GtkWidget *mistakelabel;
	GtkWidget *keystrokelabel;
	GtkWidget *timerlabel;
	GTimer *timer;
} GeditTypingTutorPlugin;

/*
 * Class
 */

typedef struct _GeditTypingTutorPluginClass
{
	GeditPluginClass parent_class;
} GeditTypingTutorPluginClass;

GType gedit_typingtutor_plugin_get_type(void) G_GNUC_CONST;

/*
 * casting macros
 */

#define GEDIT_TYPE_TYPINGTUTOR_PLUGIN			\
	(gedit_typingtutor_plugin_get_type())

#define GEDIT_TYPINGTUTOR_PLUGIN(object)		\
	(G_TYPE_CHECK_INSTANCE_CAST((object),		\
		GEDIT_TYPE_TYPINGTUTOR_PLUGIN,		\
		GeditTypingTutorPlugin))

#define GEDIT_TYPINGTUTOR_PLUGIN_CLASS(klass)		\
	(G_TYPE_CHECK_CLASS_CAST((klass),		\
		GEDIT_TYPE_TYPINGTUTOR_PLUGIN,		\
		GeditTypingTutorPluginClass))

#define GEDIT_IS_TYPINGTUTOR_PLUGIN(object)		\
	(G_TYPE_CHECK_INSTANCE_TYPE((object),		\
		GEDIT_TYPE_TYPINGTUTOR_PLUGIN))

#define GEDIT_IS_TYPINGTUTOR_PLUGIN_CLASS(object)	\
	(G_TYPE_CHECK_CLASS_TYPE((klass),		\
		GEDIT_TYPE_TYPINGTUTOR_PLUGIN))

#define GEDIT_TYPINGTUTOR_PLUGIN_GET_CLASS(object)	\
	(G_TYPE_INSTANCE_GET_CLASS((object),		\
		GEDIT_TYPE_TYPINGTUTOR_PLUGIN,		\
		GeditTypingTutorPluginClass))

/*
 * below function is necessary for gedit
 */

G_MODULE_EXPORT GType register_gedit_plugin(GTypeModule *module);

G_END_DECLS

#endif /* __GEDIT_TYPINGTUTOR_PLUGIN_H__ */
