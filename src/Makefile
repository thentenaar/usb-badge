CC=gcc
CFLAGS=-O2
VERSION=1.1
ARCH=`uname -m | sed -e 's/x86_64/amd64/' | sed -e 's/i.86/i386/g'`

all: usb-badge-gui usb-badge-cli usb-badge-test

usb-badge-test:
	$(CC) $(CFLAGS) -o usb-badge-test badge-test.c badge.c -lhid -lusb

usb-badge-gui:
	$(CC) $(CFLAGS) -o usb-badge-gui badge-gui.c badge.c bitmap_editor.c `pkg-config --cflags --libs gtk+-2.0` -lhid -lusb

usb-badge-cli:
	$(CC) $(CFLAGS) -o usb-badge-cli badge-cli.c badge.c -lhid -lusb

install: usb-badge-gui usb-badge-cli
	@mkdir -p /usr/bin
	@mkdir -p /usr/share/applications
	@mkdir -p /usr/share/usb-badge
	install -m 0755 usb-badge-gui /usr/bin/usb-badge-gui
	install -m 0755 usb-badge-cli /usr/bin/usb-badge-cli
	install -m 0644 ../etc/udev/rules.d/40-usb-badge.rules /etc/udev/rules.d/40-usb-badge.rules
	install -m 0755 ../usr/share/applications/usb-badge.desktop \
	                /usr/share/applications/usb-badge.desktop
	install -m 0755 ../usr/share/usb-badge/usb-badge.png \
	                /usr/share/usb-badge/usb-badge.png

uninstall:
	@rm -rf /usr/bin/usb-badge-gui /usr/bin/usb-badge-cli /usr/share/usb-badge /usr/share/applications/usb-badge.desktop

clean:
	@rm -f usb-badge-gui usb-badge-cli usb-badge-test ../usb-badge_$(VERSION)-1_$(ARCH).deb

deb: usb-badge-gui usb-badge-cli
	@echo "Building debian package: usb-badge_$(VERSION)-1_$(ARCH).deb..."
	@eval sed 's/@ARCH@/'$(ARCH)'/g' ../debian/control.in > ../debian/control.tmp
	@eval sed 's/@VERSION@/'$(VERSION)'/g' ../debian/control.tmp > ../debian/control
	@rm -f ../debian/control.tmp
	@mkdir -p ../usr/bin
	@install -m 0755 usb-badge-gui ../usr/bin/
	@install -m 0755 usb-badge-cli ../usr/bin/
	@md5sum ../usr/bin/usb-badge-gui > ../debian/md5sums
	@md5sum ../usr/bin/usb-badge-cli >> ../debian/md5sums
	@md5sum ../usr/share/applications/usb-badge.desktop >> ../debian/md5sums
	@md5sum ../usr/share/usb-badge/usb-badge.png >> ../debian/md5sums
	@md5sum ../etc/udev/rules.d/40-usb-badge.rules >> ../debian/md5sums
	@echo "2.0" > debian-binary
	@cd ../debian ; tar -czf ../src/control.tar.gz control md5sums
	@cd ../src ; tar -czf data.tar.gz ../usr ../etc
	@ar -r ../usb-badge_$(VERSION)-1_$(ARCH).deb \
	 debian-binary control.tar.gz data.tar.gz
	@echo "Cleaning up..."
	@rm -rf debian-binary control.tar.gz data.tar.gz ../usr/bin ../debian/control ../debian/md5sums

rpm: deb
	@alien -r ../usb-badge_$(VERSION)-1_$(ARCH).deb
	@mv usb-badge-$(VERSION)-2.$(ARCH).rpm ..

tgz: deb
	@alien -t ../usb-badge_$(VERSION)-1_$(ARCH).deb
	@mv usb-badge-$(VERSION).tgz ../usb-badge-$(VERSION).$(ARCH).tgz

