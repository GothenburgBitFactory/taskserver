CFLAGS  = -I. -I/usr/local/include -Wall -pedantic -D HAVE_LIBGNUTLS=1
LFLAGS  = -L/usr/local/lib
LIBS    = -lgnutls

all: c s

clean:
	-rm *.o s c

c: c.o TLSClient.o
	g++ c.o TLSClient.o $(LFLAGS) $(LIBS) -o c

s: s.o TLSServer.o
	g++ s.o TLSServer.o $(LFLAGS) $(LIBS) -o s

.cpp.o:
	g++ -c $(CFLAGS) $<
