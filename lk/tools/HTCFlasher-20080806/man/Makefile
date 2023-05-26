all:
	pod2man -c "User commands" -n yang yang.pod |sed -e 's/"perl.*/"yang v1.1" "User commands"/g' > yang.1
	pod2man -c "User commands" -n HTCFlasher HTCFlasher.pod |sed -e 's/"perl.*/"HTCFlasher v3.1" "User commands"/g' > HTCFlasher.1
	pod2man -c "User commands" -n HTCFlasherGUI HTCFlasherGUI.pod |sed -e 's/"perl.*/"HTCFlasherGUI v3.1" "User commands"/g' > HTCFlasherGUI.1

install:
	install -m 444 HTCFlasher.1 /usr/share/man/man1/
	install -m 444 HTCFlasherGUI.1 /usr/share/man/man1/
	install -m 444 yang.1 /usr/share/man/man1/
