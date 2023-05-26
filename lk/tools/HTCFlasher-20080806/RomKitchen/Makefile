CC?=gcc
CFLAGS=-m32 -Wall

all:
	${CC} dsmbuild.c -o dsmbuild ${CFLAGS}
	${CC} dsminfo.c -o dsminfo ${CFLAGS}
	${CC} sofbuild.c -o sofbuild ${CFLAGS}
	${CC} nbimg.c -o nbimg ${CFLAGS}
	${CC} nbsplit.c -o nbsplit ${CFLAGS}
	${CC} ImgfsFromNb.c -o ImgfsFromNb ${CFLAGS}

install:
	cp dsmbuild /usr/bin/
	cp sofbuild /usr/bin/
	cp dsminfo /usr/bin/
	cp nbimg /usr/bin/
	cp nbsplit /usr/bin/
	cp ImgfsFromNb /usr/bin/
	cp lrk /usr/bin/
	cp lrk-extract /usr/bin/
	cp lrk-build /usr/bin/
	mkdir -p /usr/share/LinuxRomKitchen/
	cp lrk-packages /usr/share/LinuxRomKitchen/
	cp lrk-scripts /usr/share/LinuxRomKitchen/
	cp lrk-preload /usr/share/LinuxRomKitchen/
	cp lrk.svg /usr/share/LinuxRomKitchen/
	cp lrk.desktop /usr/share/applications/

clean:
	-rm -f dsmbuild dsminfo sofbuild nbimg nbsplit ImgfsFromNb

remove:
	rm /usr/bin/dsmbuild
	rm /usr/bin/dsminfo
	rm /usr/bin/sofbuild
	rm /usr/bin/nbimg
	rm /usr/bin/nbsplit
	rm /usr/bin/ImgfsFromNb
	rm /usr/bin/lrk
	rm /usr/bin/lrk-extract
	rm /usr/bin/lrk-build
	rm -rf /usr/share/LinuxRomKitchen/
