noshout: noshout.c
	gcc -o $@ $< -lpulse -lpulse-mainloop-glib

install: noshout
	mkdir --parents ${DESTDIR}/usr/bin
	cp ./$< ${DESTDIR}/usr/bin
