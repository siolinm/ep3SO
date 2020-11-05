DIRNAME    = ep3-lucas-marcos
EPOBJS     = ep3.o util.o
CFLAGS     = -Wall -g -pedantic -Wno-unused-result
LIBS_EP3   = # -lpthread

default: ep3

ep3: $(EPOBJS)
	gcc $(CFLAGS) $(EPOBJS) -o ep3 $(LIBS_EP3)

%.o: %.c %.h util.h
	gcc $(CFLAGS) -c $<

tar:
	rm -Rf ../$(DIRNAME)
	rm -f ../$(DIRNAME).tar.gz
	mkdir ../$(DIRNAME)

	# copia os slides, o LEIAME e o Makefile
	cp ../LEIAME ../$(DIRNAME)/LEIAME
	cp Makefile ../$(DIRNAME)/Makefile
	cp ../slides/slides.pdf ../$(DIRNAME)/slides.pdf

	# copia o codigo fonte
	for a in *.c; do \
		cp $$a ../$(DIRNAME)/$$a; \
	done

	# (se houverem .h)
	# for a in *.h; do \
	# 	cp $$a ../$(DIRNAME)/$$a; \
	# done

	# gera ../$(DIRNAME).tar.gz
	tar -czvf ../$(DIRNAME).tar.gz ../$(DIRNAME)
	rm -Rf ../$(DIRNAME)


clean:
	rm *.o ep3-lucas-marcos *~
