#
# usb-badge
# Copyright (C) 2009-2016 Tim Hentenaar.
#
# This code is licenced under the Simplified BSD License.
# See the LICENSE file for details.
#

noinst_HEADERS  = badge.h icon.h bitmap_editor.h
bin_PROGRAMS    = usb-badge-cli
noinst_PROGRAMS = usb-badge-test

if BUILD_HIDAPI
HID_CPPFLAGS = -I$(top_srcdir)/hidapi
HID_LIBS     = $(top_srcdir)/hidapi/$(HIDAPI_OS)/libhidapi$(HIDAPI_TARGET).la
else
HID_CPPFLAGS =
HID_LIBS     = -lhidapi$(HIDAPI_TARGET)
endif

if BUILD_GUI
bin_PROGRAMS += usb-badge-gui
usb_badge_gui_SOURCES = gui.c bitmap_editor.c badge.c
usb_badge_gui_CFLAGS  = $(GTK2_CFLAGS) $(HID_CPPFLAGS)\
                        -isystem /usr/include/glib-2.0\
                        -isystem /usr/include/gtk-2.0\
                        -Wno-deprecated-declarations
usb_badge_gui_LDADD   = $(GTK2_LIBS) $(HID_LIBS)
endif

usb_badge_cli_CFLAGS  = $(HID_CPPFLAGS)
usb_badge_cli_SOURCES = cli.c badge.c
usb_badge_cli_LDADD   = $(HID_LIBS)

usb_badge_test_CFLAGS  = $(HID_CPPFLAGS)
usb_badge_test_SOURCES = test.c badge.c
usb_badge_test_LDADD   = $(HID_LIBS)

