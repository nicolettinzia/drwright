INCLUDES =					\
	@DRWRIGHT_CFLAGS@			\
	-DGLADEDIR=\"$(datadir)/drwright\"	\
	-DIMAGEDIR=\"$(datadir)/drwright\"	\
	-DDBUS_API_SUBJECT_TO_CHANGE=1

if HAVE_DBUS
dbus_sources = dbus.c dbus.h
endif

bin_PROGRAMS = drwright

drwright_SOURCES = 				\
	$(dbus_sources)				\
	main.c					\
	drwright.c				\
	drwright.h				\
	drw-break-window.c			\
	drw-break-window.h			\
	drw-utils.c				\
	drw-utils.h				\
	drw-monitor.c				\
	drw-monitor.h				\
	drw-preferences.c			\
	drw-preferences.h			\
	eggtrayicon.c				\
	eggtrayicon.h				\
	drw-selection.c				\
	drw-selection.h

drwright_LDADD = @DRWRIGHT_LIBS@ @XSS_LIBS@

schemadir = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = drwright.schemas

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)

EXTRA_DIST = $(schema_DATA) $(dbus_sources)
