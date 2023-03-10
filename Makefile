BASEDIR = ./deps/reef/
include $(BASEDIR)Make.env

SOURCES := $(wildcard *.c)
OBJECTS := $(SOURCES:.c=.o)
DEPEND	= .depend

INCS = $(INCBASE)
LIBS = $(LIBBASE)

all: aword

$(DEPEND): $(SOURCES)
	@$(CC) $(CFLAGS) $(INCS) -MM $^ > $@

include $(DEPEND)
%.o: %.c
	$(CC) $(CFLAGS) $(INCS) -o $@ -c $<

aword: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(OBJECTS) aword
