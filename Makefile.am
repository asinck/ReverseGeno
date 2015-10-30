lib_LTLIBRARIES = ReverseGeno.la

ReverseGeno_la_SOURCES = ReverseGeno.cpp
ReverseGeno_la_CPPFLAGS= -I$(top_srcdir)/include -I$(top_srcdir)/plugins/plugin_utils
ReverseGeno_la_LDFLAGS = -module -avoid-version -shared
ReverseGeno_la_LIBADD = $(top_builddir)/plugins/plugin_utils/libplugin_utils.la

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.ReverseGeno.txt \
	ReverseGeno.def \
	ReverseGeno.sln \
	ReverseGeno.vcxproj

MAINTAINERCLEANFILES =	\
	Makefile.in
