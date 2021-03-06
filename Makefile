# Makefile.in for IO ERROR's ISCA BBS Client 2.3.0+
# Copyright (C) 2001 Michael Hampton.  GNU GPL version 2 or later.

CC=gcc
CFLAGS=-g -O2
LDFLAGS=
LIBS=-lutil -lresolv 
srcdir=.

prefix=/usr/local
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
mandir=${prefix}/man
INSTALL=/usr/bin/install -c
INSTALL_DATA=${INSTALL} -m 644
INSTALL_PROGRAM=${INSTALL}

all: bbs

.SUFFIXES: .c .o

OBJS=bbsrc.o color.o config.o edit.o filter.o getline.o global.o\
	info.o inkey.o main.o queue.o slist.o sysio.o telnet.o utility.o
SOCKS_OBJS=socks/porttoserv.o socks/Rconnect.o socks/Rgethostbyname.o socks/SendGetDst.o socks/shell_cmd.o socks/socks_ckcf.o socks/socks_rdconf.o socks/socks_rdfz.o socks/utils.o
PLATFORM_OBJS=unix.o

clean:
	rm -f *.o socks/*.o bbs core core.*

distclean: clean
	rm -f Makefile config.cache config.log config.status config.h *~

maintainer-clean: distclean
	rm -f configure config.sub config.guess install-sh -r autom4te*.cache

Makefile: $(srcdir)/Makefile.in
	CONFIG_FILES=Makefile CONFIG_HEADERS= ./config.status

config.h: $(srcdir)/config.h.in
	CONFIG_FILES= CONFIG_HEADERS=config.h ./config.status

configure: $(srcdir)/configure.ac
	autoconf
	autoheader

install-exec: all
	$(INSTALL_PROGRAM) bbs $(bindir)

install-data: all
	$(INSTALL_DATA) bbs.1 $(mandir)

install: install-exec install-data
	@echo 'Now hot Asian sluts will come out of your computer whenever you type "bbs".'

bbs: $(OBJS) $(SOCKS_OBJS) $(PLATFORM_OBJS)
	$(CC) -o bbs $(OBJS) $(SOCKS_OBJS) $(PLATFORM_OBJS) $(LDFLAGS) $(LIBS)
	@echo "Congratulations, you're the proud owner of a brand new bag of bits."

.c.o:
	$(CC) $(CFLAGS) $(DEFS) -c $(srcdir)/$< -o $@

$(OBJS): config.h $(srcdir)/defs.h $(srcdir)/proto.h $(srcdir)/ext.h $(srcdir)/sysio.h
$(SOCKS_OBJS): $(srcdir)/defs.h $(srcdir)/ext.h $(srcdir)/socks/socks.h
bbsrc.o: $(srcdir)/bbsrc.c
color.o: $(srcdir)/color.c
config.o: $(srcdir)/config.c
edit.o: $(srcdir)/edit.c
filter.o: $(srcdir)/filter.c
getline.o: $(srcdir)/getline.c
global.o: $(srcdir)/global.c
info.o: $(srcdir)/info.c
inkey.o: $(srcdir)/inkey.c
main.o: $(srcdir)/main.c
queue.o: $(srcdir)/queue.c
slist.o: $(srcdir)/slist.c
sysio.o: $(srcdir)/sysio.c
telnet.o: $(srcdir)/telnet.c $(srcdir)/telnet.h
utility.o: $(srcdir)/utility.c
unix.o: $(srcdir)/unix.c $(srcdir)/unix.h config.h $(srcdir)/defs.h $(srcdir)/proto.h $(srcdir)/ext.h $(srcdir)/sysio.h
