#include <efi.h>
#include <efilib.h>
#include <stdbool.h>

void HLT();

bool var=false;

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    Print(L"Hello, UEFI World!\n");
    //hlt the application
    HLT();

    return EFI_SUCCESS;
}

void HLT(){
    Print(L"halting...\n");
    // This function halts the application by entering an infinite loop.
    while (1) {
        // The HLT instruction is used to halt the CPU until the next interrupt.
        __asm__ __volatile__("hlt");

        if (var){
            break;
        }
    }
}