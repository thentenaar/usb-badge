usb-badge
==========

A simple program for manupulating the FURI KEYSHINE USB Bage
(0x04d9:0xe002). I've included both a CLI utility (``usb-badge-cli``),
and a GUI utility (``usb-badge-gui``). The CLI was originally contributed by
[Jeff Jahr](http://www.last-outpost.com/~malakai/dcled), and some patches were
contributed by [Cody Boisclair](http://www.zone38.net/).

The GUI requires GTK+ >= 2.14.0.

Synopsis
--------

```
USB Badge CLI
Copyright (C) 2009-2016 Tim Hentenaar

Usage: usb-badge-cli [options...]

Options:
        -h Show this message
        -d Dump message data. Valid values are: 1 - Dump, 0 - Don't.
        -l Set the brighness of the display. Valid values are 0-7.
        -i Index of message to dump or modify. Valid values are 0-4.
        -a Set the display action. Valid values are:
                0 - Move                    1 - Flash, then move
                2 - Scroll Up               3 - Scroll Down
                4 - Flash                   5 - Freeze

        -s Set the update speed of the message. Valid values are 0-7.
        -m Set the message text (136 chars max.)
        -x Set the message data as a hexadecimal string (136 bytes max.)

Examples:
        Dumping all message data:     src/usb-badge-cli -d
        Dumping a specific message:   src/usb-badge-cli -d -i <index>
        Setting luminance:            src/usb-badge-cli -l 2
        Setting speed/action:         src/usb-badge-cli -i <index> -s 2 -a 1
        Updating message text:        src/usb-badge-cli -i <index> -m Message
```

Licensing
---------

See the [LICENSE](./LICENSE) file for details. The bundled hidapi is
distributed under its included BSD license.

Installation
------------

Building is as simple as:
```
$ ./autogen.sh && make && make install
```

Note that you can pass arguments to ``configure`` via autogen.sh. for
example:
```
$ ./autogen.sh --prefix=/usr --sysconfdir=/etc
```

On Linux, udev rules for the converter will also be installed in
``$(sysconfdir)/udev/rules.d`` which will give cause the badge to be
owned by the ``plugdev`` group, with permissions of 0660. Thus, you may
want to pass ``--sysconfdir=/etc`` to configure.

