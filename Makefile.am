#
# usb-badge
# Copyright (C) 2009-2016 Tim Hentenaar.
#
# This code is licenced under the Simplified BSD License.
# See the LICENSE file for details.
#

EXTRA_DIST = README.md LICENSE m4 doc

if UDEV
udevdir   = $(sysconfdir)/udev/rules.d
udev_DATA = udev/99-usb-badge.rules
endif

if BUILD_GUI
desktopdir   = $(datadir)/applications
desktop_DATA = usb-badge.desktop

icondir      = $(datadir)/usb-badge
icon_DATA    = usb-badge.png
endif

SUBDIRS = $(HIDAPI_SUBDIR) src
ACLOCAL_AMFLAGS = -I m4

