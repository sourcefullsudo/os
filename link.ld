OUTPUT_FORMAT("efi-app-x86_64")
ENTRY(efi_main)

SECTIONS {
    . = 0;
    .text : { *(.text*) }
    .data : { *(.data*) }
    .bss  : { *(.bss*)  }
}

