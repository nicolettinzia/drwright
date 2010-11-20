SUBDIRS = src po data

desktop_in_files=drwright.desktop.in
desktop_files=$(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

Applicationsdir = $(datadir)/applications
Applications_DATA = $(desktop_files)

INTLTOOL = 				\
	intltool-extract.in 		\
	intltool-merge.in 		\
	intltool-update.in

EXTRA_DIST = 				\
	AUTHORS 			\
	ChangeLog			\
	README				\
	drwright.spec			\
	drwright.spec.in		\
	$(INTLTOOL)			\
	$(desktop_in_files)		\
	$(desktop_files)		\
	autogen.sh			\
	drwright.doap

DISTCLEANFILES = 		\
	intltool-extract	\
	intltool-merge		\
	intltool-update		\
	$(desktop_files)

DISTCHECK_CONFIGURE_FLAGS = --enable-dbus=no
