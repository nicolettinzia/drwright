INCLUDES =					\
	@DRWRIGHT_CFLAGS@			\
	-DGLADEDIR=\"$(datadir)/drwright\"	\
	-DIMAGEDIR=\"$(datadir)/drwright\"	\
	-Werror -Wall

bin_PROGRAMS = drwright

drwright_SOURCES = 				\
	main.c					\
	drwright.c				\
	drwright.h				\
	drw-break-window.c			\
	drw-break-window.h			\
	drw-monitor.c				\
	drw-monitor.h				\
	drw-preferences.c			\
	drw-preferences.h			\
	drw-intl.h				\
	eggtrayicon.c				\
	eggtrayicon.h				\
	drw-selection.c				\
	drw-selection.h				\
	disclosure-widget.c			\
	disclosure-widget.h

drwright_LDADD = @DRWRIGHT_LIBS@ -L/usr/X11R6/lib -lXss
drwright_LDFLAGS = -export-dynamic

imagedir = $(datadir)/drwright
image_DATA = stop.png bar.png bar-red.png bar-green.png bar-disabled.png ocean-stripes.png

schemadir = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = drwright.schemas

gladedir = $(datadir)/drwright
glade_DATA = drw-preferences.glade

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)

EXTRA_DIST = $(schema_DATA) $(image_DATA) $(glade_DATA)
