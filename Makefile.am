SUBDIRS = src po data

@INTLTOOL_DESKTOP_RULE@

desktop_in_files=drwright.desktop.in
desktop_files=$(desktop_in_files:.desktop.in=.desktop)

Applicationsdir = $(datadir)/applications
Applications_DATA = $(desktop_files)

EXTRA_DIST = 				\
	AUTHORS 			\
	ChangeLog			\
	README				\
	drwright.spec			\
	drwright.spec.in		\
	intltool-extract.in  		\
	intltool-merge.in    		\
	intltool-update.in		\
	$(desktop_in_files)		\
	$(desktop_files)

