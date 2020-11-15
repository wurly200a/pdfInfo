TARGET = pdfInfo.exe
CC = gcc

#CFLAGS = -I../inc -O1 -Wall -fmessage-length=0 -finput-charset=cp932
CFLAGS = -I../inc -O1 -Wall -fmessage-length=0

CPPFLAGS = -c

LDFLAGS = -Wl,--subsystem,console
LDLIBS =

OBJECTS = \
          ..\\main.o       \
          ..\\pdf\pdf.o    \
          ..\\pdf\pdfLib.o \

$(TARGET): $(OBJECTS)
		$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
		$(RM) $(OBJECTS)
		$(RM) $(TARGET)

.c.o:
		$(CC) $(CFLAGS) $(CPPFLAGS) $< -o $@
