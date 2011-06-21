OBJS =  pdefault.o psndread.o pplay.o pacgfx.o pextras.o pghost2.o \
 ptree.o popen.o pread.o pscreen.o pmenus.o pgame.o pm.o pcon.o pcon2.o \
 pwrite.o

pm:	$(OBJS)
	$(CC) -o pm $(OBJS) -lvga -lm

run:	pm
	pm

pdefault.o: pdefault.c
psndread.o: psndread.c
pplay.o: pplay.c
pacgfx.o: pacgfx.c
pextras.o: pextras.c
pghost2.o: pghost2.c
ptree.o: ptree.c
popen.o: popen.c keydef.h
pread.o: pread.c
pscreen.o: pscreen.c
pcon.o: pcon.c pgadget.h pcon.h
pcon2.o: pcon2.c pgadget.h pcon.h
pgame.o: pgame.c
pwrite.o: pwrite.c
pmenus.o: pmenus.c pmenus.h
pm.o: pm.c pacvar.h pacvar2.h pacdef.h keydef.h
