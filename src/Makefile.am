NULL =

# Typing break

bin_PROGRAMS = gnome-typing-monitor

gnome_typing_monitor_SOURCES =			\
	main.c					\
	drwright.c				\
	drwright.h				\
	drw-break-window.c			\
	drw-break-window.h			\
	drw-monitor.c				\
	drw-monitor.h				\
	drw-utils.c				\
	drw-utils.h				\
	drw-selection.c				\
	drw-selection.h				\
	drw-timer.c				\
	drw-timer.h

gnome_typing_monitor_CPPFLAGS = \
	-DGNOMELOCALEDIR="\"$(datadir)/locale\""	\
	-DIMAGEDIR=\"$(pkgdatadir)/pixmaps\"		\
	$(AM_CPPFLAGS)
gnome_typing_monitor_CFLAGS = \
	$(TYPING_CFLAGS)	\
	$(AM_CFLAGS)

gnome_typing_monitor_LDADD = $(TYPING_LIBS) $(SCREENSAVER_LIBS)

if HAVE_APP_INDICATOR
  gnome_typing_monitor_CFLAGS += -DHAVE_APP_INDICATOR $(APP_INDICATOR_CFLAGS)
  gnome_typing_monitor_LDADD += $(APP_INDICATOR_LIBS)
endif

imagedir = $(pkgdatadir)/pixmaps
dist_image_DATA = bar.png bar-red.png bar-green.png bar-disabled.png ocean-stripes.png

# Themeable application icon
icondir = $(datadir)/icons/hicolor/48x48/apps
dist_icon_DATA = typing-monitor.png
svgicondir = $(datadir)/icons/hicolor/scalable/apps
dist_svgicon_DATA = typing-monitor.svg
gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor
install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi


# GSD plugin

gsd_plugin_LTLIBRARIES = \
	libtyping-break.la		\
	$(NULL)

libtyping_break_la_SOURCES = 		\
	gsd-typing-break-plugin.h	\
	gsd-typing-break-plugin.c	\
	gsd-typing-break-manager.h	\
	gsd-typing-break-manager.c	\
	gnome-settings-profile.h	\
	$(NULL)

libtyping_break_la_CPPFLAGS = \
	-DGNOME_SETTINGS_LOCALEDIR=\""$(datadir)/locale"\" \
	$(AM_CPPFLAGS)

libtyping_break_la_CFLAGS = \
	$(GSD_PLUGIN_CFLAGS)	\
	$(AM_CFLAGS)

libtyping_break_la_LDFLAGS = 		\
	$(GSD_PLUGIN_LDFLAGS)	\
	$(NULL)

libtyping_break_la_LIBADD  = 		\
	$(GSD_PLUGIN_LIBS)	\
	$(NULL)

gsd_plugin_in_files = 		\
	typing-break.gnome-settings-plugin.in	\
	$(NULL)

gsd_plugin_DATA = $(gsd_plugin_in_files:.gnome-settings-plugin.in=.gnome-settings-plugin)

@GSD_INTLTOOL_PLUGIN_RULE@

# GConf schema

schemadir = $(GCONF_SCHEMA_FILE_DIR)
schema_in_files = drwright.schemas.in
schema_DATA = drwright.schemas

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/src/$(schema_DATA)
endif

@INTLTOOL_SCHEMAS_RULE@

# Clean

EXTRA_DIST = 			\
	$(gsd_plugin_in_files)	\
	$(schema_in_files) 	\
	$(NULL)

CLEANFILES = 			\
	$(gsd_plugin_DATA)		\
	$(schema_DATA)		\
	$(NULL)

DISTCLEANFILES =		\
	$(gsd_plugin_DATA)	\
	$(schema_DATA)		\
	$(NULL)

# includes
 
-include $(top_srcdir)/git.mk
