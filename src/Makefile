CC=gcc
VERSION=1.0
ARCH=`uname -m | sed -e 's/x86_64/amd64/' | sed -e 's/i.86/i386/g'`

all: usb-badge-gui usb-badge-test

usb-badge-test:
	$(CC) -o usb-badge-test badge-test.c badge.c -lhid -lusb

usb-badge-gui:
	$(CC) -o usb-badge-gui badge-gui.c badge.c bitmap_editor.c `pkg-config --cflags --libs gtk+-2.0` -lhid -lusb

install: usb-badge-gui
	@mkdir -p /usr/bin
	@mkdir -p /usr/share/applications
	@mkdir -p /usr/share/usb-badge
	install -m 0755 usb-badge-gui /usr/bin/usb-badge-gui
	install -m 0755 ../usr/share/applications/usb-badge.desktop \
	                /usr/share/applications/usb-badge.desktop
	install -m 0755 ../usr/share/usb-badge/usb-badge.png \
	                /usr/share/usb-badge/usb-badge.png

uninstall:
	@rm -rf /usr/bin/usb-badge-gui /usr/share/usb-badge /usr/share/applications/usb-badge.desktop

clean:
	@rm -f usb-badge-gui usb-badge-test ../usb-badge_$(VERSION)-1_$(ARCH).deb

deb: usb-badge-gui
	@echo "Building debian package: usb-badge_$(VERSION)-1_$(ARCH).deb..."
	@eval sed 's/@ARCH@/'$(ARCH)'/g' ../debian/control.in > ../debian/control
	@mkdir -p ../usr/bin
	@install -m 0755 usb-badge-gui ../usr/bin/
	@md5sum ../usr/bin/usb-badge-gui > ../debian/md5sums
	@md5sum ../usr/share/applications/usb-badge.desktop >> ../debian/md5sums
	@md5sum ../usr/share/usb-badge/usb-badge.png >> ../debian/md5sums
	@echo "2.0" > debian-binary
	@cd ../debian ; tar -czf ../src/control.tar.gz control md5sums
	@cd ../src ; tar -czf data.tar.gz ../usr
	@ar -r ../usb-badge_$(VERSION)-1_$(ARCH).deb \
	 debian-binary control.tar.gz data.tar.gz
	@echo "Cleaning up..."
	@rm -rf debian-binary control.tar.gz data.tar.gz ../usr/bin ../debian/control

rpm: deb
	@alien -r ../usb-badge_$(VERSION)-1_$(ARCH).deb
	@mv usb-badge-$(VERSION)-2.$(ARCH).rpm ..

tgz: deb
	@alien -t ../usb-badge_$(VERSION)-1_$(ARCH).deb
	@mv usb-badge-$(VERSION).tgz ../usb-badge-$(VERSION).$(ARCH).tgz

