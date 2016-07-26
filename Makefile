## makefile for gxtuner by @brummer
## please edit to your needs
## set the install Path for gxtuner

    #@defines
	PREFIX = /usr
	BIN_DIR = $(PREFIX)/bin
	SHARE_DIR = $(PREFIX)/share
	DESKAPPS_DIR = $(SHARE_DIR)/applications
	PIXMAPS_DIR = $(SHARE_DIR)/pixmaps
	INCLUDE_DIR = /usr/include/
	INCLUDE_L_DIR = /usr/local/include/
	VER = 2.1
	NAME = gxtuner
	LIBS = `pkg-config --libs jack gtk+-2.0 gthread-2.0 fftw3f x11` -lzita-resampler
	CFLAGS += -Wall -ffast-math `pkg-config --cflags jack gtk+-2.0 gthread-2.0 fftw3f`
	OBJS = jacktuner.o gxtuner.o cmdparser.o pitchtracker.o gtkknob.o \
           paintbox.o tuner.o deskpager.o main.o
	DEBNAME = $(NAME)_$(VER)
	CREATEDEB = yes '' | dh_make -s -n -e $(USER)@org -p $(DEBNAME) -c gpl >/dev/null
	DIRS = $(BIN_DIR)  $(DESKAPPS_DIR)  $(PIXMAPS_DIR) 
	BUILDDEB = dpkg-buildpackage -rfakeroot -b 2>/dev/null | grep dpkg-deb 
	#SOURCES =$(OBJS:%.o=%.cpp)

	## output style (bash colours)
	BLUE = "\033[1;34m"
	BROWN = "\033[0;33m"
	GREEN = "\033[0;32m"
	LGREEN = "\033[1;32m"
	RED =  "\033[1;31m"
	NONE = "\033[0m"

    #@default build with jack session support
all : config
	@$(MAKE) check

    #@build without jack session support
nosession : nconf
	@$(MAKE) check

    #@link object files to build executable
link : $(OBJS)
	@rm -rf $(NAME)
	@echo $(BROWN)
	- $(CXX) $(LDFLAGS) $(CFLAGS) $(CPPFLAGS) $(OBJS) $(LIBS) -o $(NAME)

    #@check if build have worked
check : link
	@if [ -f ./$(NAME) ]; then echo $(BLUE)"build finish, now run make install"; \
	else echo $(RED)"sorry, build failed"; fi
	@echo $(NONE)

    #@create resampler.h to set the used zita-resamper version
resamp:
	-@if [ -f ./resample.h ] ; then \
    echo ''; else \
	if [ -f $(INCLUDE_DIR)zita-resampler/resampler.h 2>/dev/null ]; then \
	echo '#include <zita-resampler/resampler.h>' > resample.h; else \
	if [ -f $(INCLUDE_DIR)zita-resampler.h 2>/dev/null ]; then \
	echo '#include <zita-resampler.h>' > resample.h; else \
	if [ -f $(INCLUDE_L_DIR)zita-resampler/resampler.h 2>/dev/null ]; then \
	echo '#include <zita-resampler/resampler.h>' > resample.h; else \
	if [ -f $(INCLUDE_L_DIR)zita-resampler.h 2>/dev/null ]; then \
	echo '#include <zita-resampler.h>' > resample.h; else \
	echo  $(RED)"error: zita-resampler library not found"$(GREEN); fi; fi; fi; fi; fi;
	@if [ -f resample.h ]; then echo  $(LGREEN)"zita-resampler library ok"$(GREEN); \
	else exit 2; fi; 

    #@create config.h for build with jack session support
config : resamp
	-@if cat config.h 2>/dev/null | grep HAVE_JACK_SESSION >/dev/null; then \
    echo ''; else \
    echo '#define VERSION  "$(VER)"' > config.h ;\
    echo '#define PIXMAPS_DIR  "$(PIXMAPS_DIR)"' >> config.h ;\
    echo '#ifndef HAVE_JACK_SESSION' >> config.h ;\
    echo '#define HAVE_JACK_SESSION 1' >> config.h ; \
    echo '#endif' >> config.h  ;\
    fi;
	@echo  $(LGREEN)"build $(NAME) with jack_session support"$(GREEN)
	@echo  ""

    #@create config.h for build without jack session support
nconf : resamp
	-@if [ -f ./config.h 2>/dev/null ] ; then \
    if cat config.h 2>/dev/null | grep HAVE_JACK_SESSION >/dev/null; then \
    echo '#define VERSION  "$(VER)"' > config.h ;\
    echo '#define PIXMAPS_DIR  "$(PIXMAPS_DIR)"' >> config.h ;\
    fi; else \
    echo '#define VERSION  "$(VER)"' > config.h ;\
    echo '#define PIXMAPS_DIR  "$(PIXMAPS_DIR)"' >> config.h ;\
    fi;
	@echo $(RED)"build $(NAME) without jack_session support !!"$(GREEN)
	@echo  ""

    #@build object files
jacktuner.o : jacktuner.cpp jacktuner.h config.h
	@rm -rf jacktuner.o
	-$(CXX) $(LDFLAGS) $(CFLAGS) $(CPPFLAGS) -c jacktuner.cpp

gxtuner.o : gxtuner.cpp gxtuner.h 
	@rm -rf gxtuner.o
	-$(CXX) $(LDFLAGS) $(CFLAGS) $(CPPFLAGS) -c gxtuner.cpp

cmdparser.o : cmdparser.cpp cmdparser.h config.h
	@rm -rf cmdparser.o
	-$(CXX) $(LDFLAGS) $(CFLAGS) -c cmdparser.cpp

pitchtracker.o : pitchtracker.cpp pitchtracker.h resample.h
	@rm -rf pitchtracker.o
	-$(CXX) $(LDFLAGS) $(CFLAGS) $(CPPFLAGS) -c pitchtracker.cpp

gtkknob.o : gtkknob.cc gtkknob.h
	@rm -rf gtkknob.o
	-$(CXX) $(LDFLAGS) $(CFLAGS) $(CPPFLAGS) -c gtkknob.cc

tuner.o :tuner.cpp tuner.h config.h paintbox.h gtkknob.h gxtuner.h deskpager.h
	@rm -rf tuner.o
	-$(CXX) $(LDFLAGS) $(CFLAGS) $(CPPFLAGS) -c tuner.cpp

paintbox.o : paintbox.cpp paintbox.h frame.h
	@rm -rf paintbox.o
	-$(CXX) $(LDFLAGS) $(CFLAGS) $(CPPFLAGS) -c paintbox.cpp

deskpager.o : deskpager.cpp deskpager.h
	@rm -rf deskpager.o
	-$(CXX) $(LDFLAGS) $(CFLAGS) $(CPPFLAGS) -c deskpager.cpp

main.o : main.cpp jacktuner.h gxtuner.h cmdparser.h pitchtracker.h tuner.h deskpager.h
	@rm -rf main.o
	-$(CXX) $(LDFLAGS) $(CFLAGS) $(CPPFLAGS) -c main.cpp

    #@install all
install :
	@mkdir -p $(DESTDIR)$(BIN_DIR)
	@mkdir -p $(DESTDIR)$(DESKAPPS_DIR)
	@mkdir -p $(DESTDIR)$(PIXMAPS_DIR)
	install $(NAME) $(DESTDIR)$(BIN_DIR)
	install $(NAME).desktop $(DESTDIR)$(DESKAPPS_DIR)
	install $(NAME).png $(DESTDIR)$(PIXMAPS_DIR)
	@echo $(BLUE)"$(NAME) installation finish,"$(NONE)

    #@well, clean up the build
clean :
	@echo $(RED)"clean up,"
	@rm -rf $(NAME) config.h resample.h *-stamp *~ *.o
	@echo ". ." $(BLUE)", done"$(NONE)

    #@clean up included the debian folder
clean-full :
	@echo $(RED)"clean up,"
	@rm -rf gxtuner config.h resample.h *-stamp *~ *.o
	@rm -rf ./debian/*.log ./debian/*.substvars ./debian/gxtuner
	@echo ". ." $(BLUE)", done"$(NONE)

    #@create tar ball
tar : clean-full
	@cd ../ && \
	tar -cf $(NAME)-$(VER).tar.bz2 --bzip2 gxtuner
	@echo $(LGREEN)"build gxtuner-"$(VER)".tar.bz2"$(NONE)

    #@create a tar ball, exclude ./debian
dist-tar : clean
	@cd ../ && \
	tar -cf $(NAME)-$(VER)-nopkg.tar.bz2 --bzip2 gxtuner --exclude=debian
	@echo $(LGREEN)"build gxtuner-"$(VER)"-nopkg.tar.bz2"$(NONE)

    #@build a debian packet for all arch
deb : 
	@rm -rf ./debian
	@echo $(BLUE)"create ./debian"$(NONE)
	-@ $(CREATEDEB)
	@ #@echo $(BLUE)"touch ./debian/dirs"$(NONE)
	@ #-@echo $(DIRS) > ./debian/dirs
	@echo $(BLUE)"try to build debian package, that may take some time"$(LGREEN)
	-@ if $(BUILDDEB); then echo ". ." $(BLUE)", done"$(NONE); else \
     echo $(RED)"sorry, build fail"$(NONE); fi
	@rm -rf ./debian

    #@remove the installed stuff
uninstall :
	rm -rf $(BIN_DIR)/$(NAME) $(DESKAPPS_DIR)/$(NAME).desktop $(PIXMAPS_DIR)/$(NAME).png
	@echo $(RED)"uninstall gxtuner, . . . finish"$(NONE)

