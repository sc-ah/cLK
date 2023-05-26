all:
	cd src && ${MAKE} all

install:
	cp src/HTCFlasher /usr/bin/
	cp src/yang /usr/bin/
	cp gui/HTCFlasherGUI /usr/bin/
	cp gui/yangGUI /usr/bin/
	cp gui/htcflasher.png /usr/share/icons/
	cp gui/yang.png /usr/share/icons/
	cp gui/htcflasher.desktop /usr/share/applications/
	cp gui/yang.desktop /usr/share/applications/
	cd man && ${MAKE} install

clean:
	cd src && ${MAKE} clean

remove:
	rm /usr/bin/HTCFlasher
	rm /usr/bin/HTCFlasherGUI
	rm /usr/bin/yang
	rm /usr/bin/yangGUI
	rm /usr/share/applications/htcflasher.desktop
	rm /usr/share/applications/yang.desktop
	rm /usr/share/icons/htcflasher.png
	rm /usr/share/icons/yang.png
	rm /usr/share/man/man1/HTCFlasher.1
	rm /usr/share/man/man1/HTCFlasherGUI.1
	rm /usr/share/man/man1/yang.1

kitchen:
	cd RomKitchen && ${MAKE} all && ${MAKE} install
