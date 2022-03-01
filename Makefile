CC=gcc
CFLAGS= -s -Os -ffunction-sections -fdata-sections
LDFLAGS= -Wl,--gc-sections
VM_OBJFILES = vm_x2017_main.o vm_x2017.o objdump_x2017.o
OBJDUMP_OBJFILES = objdump_x2017.o objdump_x2017_main.o
TARGET1 = objdump_x2017
TARGET2 = vm_x2017

all: $(VM_OBJFILES) $(OBJDUMP_OBJFILES) $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJDUMP_OBJFILES)
	$(CC) $(CFLAGS) $(OBJDUMP_OBJFILES) -o $(TARGET1) $(LDFLAGS)
	strip --strip-all objdump_x2017

objdump_x2017.o: objdump_x2017.c objdump_x2017.h
	$(CC) $(CFLAGS) -c objdump_x2017.c

objdump_x2017_main.o: objdump_x2017_main.c objdump_x2017.h
	$(CC) $(CFLAGS) -c objdump_x2017_main.c

$(TARGET2): $(VM_OBJFILES)
	$(CC) $(CFLAGS)  $(VM_OBJFILES) -o $(TARGET2) $(LDFLAGS)
	strip --strip-all vm_x2017
	bash gzexe_edited.sh vm_x2017

vm_x2017_main.o: vm_x2017_main.c vm_x2017.h
	$(CC) $(CFLAGS) -c vm_x2017_main.c

vm_x2017.o: vm_x2017.c vm_x2017.h
	$(CC) $(CFLAGS) -c vm_x2017.c

test:
	bash test.sh

clean:
	rm objdump_x2017
	rm vm_x2017
	rm vm_x2017_main.o 
	rm vm_x2017.o 
	rm objdump_x2017.o
	rm objdump_x2017_main.o