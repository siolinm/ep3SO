DIRNAME    = ep3-lucas-marcos
EPOBJS     = ep3.o util.o commands.o
CFLAGS     = -Wall -g -pedantic -Wno-unused-result
COMPILER   = g++
LIBS_EP3   = # -lpthread

default: ep3

ep3: $(EPOBJS)
	$(COMPILER) $(CFLAGS) $(EPOBJS) -o ep3 $(LIBS_EP3)

%.o: %.cpp %.hpp util.hpp
	$(COMPILER) $(CFLAGS) -c $<

tar:
	rm -Rf ../$(DIRNAME)
	rm -f ../$(DIRNAME).tar.gz
	mkdir ../$(DIRNAME)

	# copia os slides, o LEIAME e o Makefile
	cp ../LEIAME ../$(DIRNAME)/LEIAME
	cp Makefile ../$(DIRNAME)/Makefile
	cp ../slides/slides.pdf ../$(DIRNAME)/slides.pdf

	# copia o codigo fonte
	for a in *.cpp; do \
		cp $$a ../$(DIRNAME)/$$a; \
	done

	# (se houverem .hpp)
	for a in *.hpp; do \
		cp $$a ../$(DIRNAME)/$$a; \
	done

	# gera ../$(DIRNAME).tar.gz
	tar -czvf ../$(DIRNAME).tar.gz ../$(DIRNAME)
	rm -Rf ../$(DIRNAME)


clean:
	rm *.o ep3 *~
