CC = i686-w64-mingw32-gcc
CPP = i686-w64-mingw32-g++
DLLTOOL = i686-w64-mingw32-dlltool

CC_64 = x86_64-w64-mingw32-gcc
CPP_64 = x86_64-w64-mingw32-g++
DLLTOOL_64 = x86_64-w64-mingw32-dlltool

STRIP = strip

CFLAGS = -O2 -pipe -fno-ident -ffunction-sections -fdata-sections -fvisibility=hidden
LDFLAGS = -static -static-libgcc -Wl,--gc-sections

CFLAGS += -DVERBOSE

OMNIMIX_VERSION = 2.0

DLLS = \
	build/32/omnimix.dll #\
#	build/64/omnimix.dll

include src/util/Makefile

UTIL_SOURCES := $(src_util:%.c=src/util/%.o)

MAIN_SOURCES = $(UTIL_SOURCES) src/festo/omnimix.o libavs.a libjubeat.a
DEF_SOURCES = imports/jubeat.def imports/avs.def

SOURCES_32 = $(foreach source,$(MAIN_SOURCES),build/32/$(source))
SOURCES_64 = $(foreach source,$(MAIN_SOURCES),build/64/$(source))

all: build $(EXES) $(DLLS)

build:
	mkdir build
	mkdir build/32 build/64
	mkdir build/32/util
	mkdir build/64/util

build/32/omnimix.dll: $(SOURCES_32)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -flto -Lbuild/32 -o $@ $^ -lavs -ljubeat -lpsapi
	$(STRIP) -R .note -R .comment $@

build/64/omnimix.dll: $(SOURCES_64)
	$(CC_64) $(CFLAGS) $(LDFLAGS) -shared -flto -Lbuild/64 -o $@ $^ -lavs -ljubeat -lpsapi
	$(STRIP) -R .note -R .comment $@

clean:
	rm -f $(EXES) $(DLLS) $(SOURCES_32) $(SOURCES_64)
	rm -f build/32/libavs.a build/32/libjubeat.a
	rm -f build/64/libavs.a build/64/libjubeat.a
	rmdir build/32/util
	rmdir build/64/util
	rmdir build/32 build/64
	rmdir build

build/32/lib%.a: %.def
	$(DLLTOOL) -p jb -d $< -l $@

build/64/lib%.a: %.def
	$(DLLTOOL_64) -p jb -d $< -l $@

build/32/%.o: %.c
	$(CC) $(CFLAGS) -s -flto -DOMNIMIX_VERSION=\"$(OMNIMIX_VERSION)\" -c -o $@ $<

build/32/%.o: %.cpp
	$(CPP) $(CFLAGS) -s -flto -Wall -c -o $@ $<

build/64/%.o: %.c
	$(CC_64) $(CFLAGS) -s -flto -DOMNIMIX_VERSION=\"$(OMNIMIX_VERSION)\" -c -o $@ $<

build/64/%.o: %.cpp
	$(CPP_64) $(CFLAGS) -s -flto -Wall -c -o $@ $<
