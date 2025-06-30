ARCH=x86_64
OBJS=kernel.o
TARGET=kernel.efi
EFIINC=/usr/include/efi
EFIINCS=-I$(EFIINC) -I$(EFIINC)/$(ARCH) -I$(EFIINC)/protocol
LIB=/usr/lib
EFILIB=/usr/lib
EFI_CRT_OBJS=$(EFILIB)/crt0-efi-$(ARCH).o
EFI_LDS=$(EFILIB)/elf_$(ARCH)_efi.lds
CFLAGS=$(EFIINCS) -fno-stack-protect -fpic -fshort-wchar -mno-red-zone -Wall -DEFI_FUNCTION_WRAPPER
LDFLAGS=-nostdlib -znocombreloc -T $(EFI_LDS) -shared -Bsymbolic -L $(EFILIB) -L $(LIB) $(EFI_CRT_OBJS)

all: $(TARGET)

kernel.so: $(OBJS)
    ld $(LDFLAGS) $(OBJS) -o $@ -lefi -lgnuefi

%.efi: %.so
    objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-$(ARCH) --subsystem=10 $^ $@

%.o: %.c
    gcc $(CFLAGS) -c $< -o $@

clean:
    rm -f $(OBJS) kernel.so kernel.efi
