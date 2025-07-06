#include <efi.h>
#include <efilib.h>

#define KERNEL_FILE L"\\kernel.bin" // Path to the kernel file in the ESP

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS Status;
    EFI_GUID FileSystemProtocol =              EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_FILE_IO_INTERFACE *FileSystem;
    EFI_FILE_HANDLE RootDir, KernelFile;
    VOID *KernelBuffer;
    UINTN KernelSize = 0;
    EFI_PHYSICAL_ADDRESS KernelAddress = 0x100000; // Suggested load address (1MB)

    // Initialize the library
    InitializeLib(ImageHandle, SystemTable);

    // Print a startup message
    Print(L"Loading kernel...\n");

    // Open the volume (EFI System Partition)
    Status = SystemTable->BootServices->HandleProtocol(
        ImageHandle, &FileSystemProtocol, (VOID **)&FileSystem);
    if (EFI_ERROR(Status)) {
        Print(L"Error: Could not open file system protocol. Status: %r\n", Status);
        return Status;
    }

    Status = FileSystem->OpenVolume(FileSystem, &RootDir);
    if (EFI_ERROR(Status)) {
        Print(L"Error: Could not open volume. Status: %r\n", Status);
        return Status;
    }

    // Open the kernel file
    Status = RootDir->Open(RootDir, &KernelFile, KERNEL_FILE, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        Print(L"Error: Could not open kernel file. Status: %r\n", Status);
        return Status;
    }

    // Get the size of the kernel file
    KernelFile->SetPosition(KernelFile, (UINT64)-1);
    UINT64 FileSize;
    Status = KernelFile->GetPosition(KernelFile, &FileSize);
    if (EFI_ERROR(Status)) {
        Print(L"Error: Could not get file size. Status: %r\n", Status);
        KernelFile->Close(KernelFile);
        return Status;
    }
    KernelSize = (UINTN)FileSize;
    KernelFile->SetPosition(KernelFile, 0);

    // Allocate memory for the kernel
    Status = SystemTable->BootServices->AllocatePages(
        AllocateAddress, EfiLoaderData, (KernelSize + 0xFFF) / 0x1000, &KernelAddress);
    if (EFI_ERROR(Status)) {
        Print(L"Error: Could not allocate memory. Status: %r\n", Status);
        KernelFile->Close(KernelFile);
        return Status;
    }
    KernelBuffer = (VOID *)KernelAddress;

    // Read the kernel file into memory
    Status = KernelFile->Read(KernelFile, &KernelSize, KernelBuffer);
    if (EFI_ERROR(Status)) {
        Print(L"Error: Could not read kernel file. Status: %r\n", Status);
        SystemTable->BootServices->FreePages(KernelAddress, (KernelSize + 0xFFF) / 0x1000);
        KernelFile->Close(KernelFile);
        return Status;
    }
    KernelFile->Close(KernelFile);

    Print(L"Kernel loaded at 0x%lx, size %d bytes\n", KernelAddress, KernelSize);

    // Get the memory map (required before exiting boot services)
    UINTN MapSize = 0, MapKey, DescriptorSize;
    UINT32 DescriptorVersion;
    EFI_MEMORY_DESCRIPTOR *MemoryMap;
    Status = SystemTable->BootServices->GetMemoryMap(&MapSize, NULL, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        Status = SystemTable->BootServices->AllocatePool(EfiLoaderData, MapSize, (VOID **)&MemoryMap);
        if (EFI_ERROR(Status)) {
            Print(L"Error: Could not allocate memory map buffer. Status: %r\n", Status);
            return Status;
        }
        Status = SystemTable->BootServices->GetMemoryMap(&MapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    }
    if (EFI_ERROR(Status)) {
        Print(L"Error: Could not get memory map. Status: %r\n", Status);
        SystemTable->BootServices->FreePool(MemoryMap);
        return Status;
    }

    // Exit boot services
    Status = SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);
    if (EFI_ERROR(Status)) {
        Print(L"Error: Could not exit boot services. Status: %r\n", Status);
        SystemTable->BootServices->FreePool(MemoryMap);
        return Status;
    }

    // Cast the kernel entry point (assuming it starts at the beginning)
    VOID (*KernelEntry)(VOID) = (VOID (*)(VOID))KernelAddress;

    // Transfer control to the kernel
    Print(L"Starting kernel...\n");
    KernelEntry();

    // Should never reach here
    return EFI_SUCCESS;
}