#
#pragma region <imports>
#pragma region "export header imports"
#include "MemoryPE.h"
#pragma endregion
#
#pragma region "platform-dependent imports"

#pragma endregion
#
#pragma region "platform-independent imports"

#pragma endregion
#
#pragma region "3rd-party imports"

#pragma endregion
#
#pragma region "shared imports"

#pragma endregion
#
#pragma region "local imports"

#pragma endregion
#pragma endregion
#
#
BOOL MemoryCreateProcessA(IN PVOID Image, IN LPSTR CommandLine, IN STARTUPINFOA* o_startup_info, IN PROCESS_INFORMATION* o_process_info) {
#pragma region <locals>
    CHAR ExecutablePath[MAX_PATH + 1];
#pragma endregion


    if (GetModuleFileNameA(NULL, ExecutablePath, MAX_PATH) <= 0) 
        return FALSE;
    else {
        return MemoryCreateProcessExA(ExecutablePath, Image, CommandLine, o_startup_info, o_process_info);
    }
}


BOOL MemoryCreateProcessW(IN PVOID Image, IN LPWSTR CommandLine, IN STARTUPINFOW* o_startup_info, IN PROCESS_INFORMATION* o_process_info) {
#pragma region <locals>
    WCHAR ExecutablePath[MAX_PATH + 1];
#pragma endregion


    if (GetModuleFileNameW(NULL, ExecutablePath, MAX_PATH) <= 0)
        return FALSE;
    else {
        return MemoryCreateProcessExW(ExecutablePath, Image, CommandLine, o_startup_info, o_process_info);
    }
}


BOOL MemoryCreateProcessExA(IN LPCSTR ExecutablePath, IN PVOID Image, IN LPSTR CommandLine, IN STARTUPINFOA* o_startup_info, IN PROCESS_INFORMATION* o_process_info) {
#pragma region <locals>
	BOOL              b_create_process = FALSE,
		              b_read_process_memory = FALSE,
		              b_write_process_memory = FALSE;
    CONTEXT           stCtx;
    PIMAGE_DOS_HEADER lpDOSHeader;
    PIMAGE_NT_HEADERS lpNTHeader;
    LPVOID            lpImageBase = NULL;
#pragma endregion


    /*
     * Check that image is a PE file;
     */
    lpDOSHeader = (PIMAGE_DOS_HEADER)(Image);
#ifdef _WIN64
    lpNTHeader = (PIMAGE_NT_HEADERS)((DWORD64)(Image)+lpDOSHeader->e_lfanew);
#else
    lpNTHeader = (PIMAGE_NT_HEADERS)((DWORD)(Image)+lpDOSHeader->e_lfanew);
#endif // _WIN64

    if (lpNTHeader->Signature != IMAGE_NT_SIGNATURE){
        goto failure;
    }

    /*
     * Create process;
     */
    b_create_process = CreateProcessA(
        ExecutablePath,
        CommandLine,
        NULL,
        NULL,
        FALSE,
        CREATE_SUSPENDED,
        NULL,
        NULL,
        o_startup_info,
        o_process_info
    );

    if (!b_create_process) {
        goto failure;
    }

    /*
     * Allocate memory for the context.
     */
    RtlSecureZeroMemory(&stCtx, sizeof stCtx);

    stCtx.ContextFlags = CONTEXT_FULL;

    if (!GetThreadContext(o_process_info->hThread, &stCtx)){
		goto failure;
    }

#ifndef _WIN64
    b_read_process_memory = ReadProcessMemory(
        o_process_info->hProcess,
        (LPCVOID)(stCtx.Ebx + 8),
        (LPVOID)(&lpImageBase),
        4,
        NULL
    );

    if (!b_read_process_memory) {
        goto failure;
    }
#endif // !_WIN64

    lpImageBase = VirtualAllocEx(
        o_process_info->hProcess,
        (LPVOID)(lpNTHeader->OptionalHeader.ImageBase),
        lpNTHeader->OptionalHeader.SizeOfImage,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE
    );

    if (NULL == lpImageBase) {
        switch (GetLastError()) {
        case ERROR_INVALID_ADDRESS: // Sometimes the start fails due to bad address, so we need to re-try.
            TerminateProcess(o_process_info->hProcess, EXIT_FAILURE);

            CloseHandle(o_process_info->hThread);
            CloseHandle(o_process_info->hProcess);

            RtlSecureZeroMemory(o_process_info, sizeof * o_process_info);

            return MemoryCreateProcessExA(ExecutablePath, Image, CommandLine, o_startup_info, o_process_info);
        default:
            goto failure;
        }
    }

    /*
     * Write the image to the process;
     */
    b_write_process_memory = WriteProcessMemory(
        o_process_info->hProcess,
        lpImageBase,
        Image,
        lpNTHeader->OptionalHeader.SizeOfHeaders,
        NULL
    );

    if (!b_write_process_memory) {
		goto failure;
    }

    for (SIZE_T iSection = 0; iSection < lpNTHeader->FileHeader.NumberOfSections; ++iSection){
        PIMAGE_SECTION_HEADER stSectionHeader;
#ifdef _WIN64
        stSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD64)(Image)+lpDOSHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS64) + sizeof(IMAGE_SECTION_HEADER) * iSection);
#else
        stSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)(Image)+lpDOSHeader->e_lfanew + 248 + (iSection * 40));
#endif // _WIN64
		b_write_process_memory = WriteProcessMemory(
			o_process_info->hProcess,
			(LPVOID)((DWORD64)(lpImageBase) + stSectionHeader->VirtualAddress),
			(LPVOID)((DWORD64)(Image)+ stSectionHeader->PointerToRawData),
			stSectionHeader->SizeOfRawData,
			NULL
		);

        if (!b_write_process_memory){
			goto failure;
        }
    }

	b_write_process_memory = WriteProcessMemory(
		o_process_info->hProcess,
#ifdef _WIN64
        (LPVOID)(stCtx.Rdx + sizeof(LPVOID) * 2),
#else
        (LPVOID)(stCtx.Ebx + 8),
#endif // _WIN64
		&lpImageBase,
		sizeof(LPVOID),
		NULL
	);

    if (!b_write_process_memory){
		goto failure;
    }

    /*
     * Move address of entry point to the eax register, set context and resume thread;
     */
#ifdef _WIN64
    stCtx.Rcx = (DWORD64)(lpImageBase)+lpNTHeader->OptionalHeader.AddressOfEntryPoint;
#else
    stCtx.Eax = (DWORD)(lpImageBase)+lpNTHeader->OptionalHeader.AddressOfEntryPoint;
#endif // _WIN64

    if (!SetThreadContext(o_process_info->hThread, &stCtx)){
        goto failure;
    }

    if (!ResumeThread(o_process_info->hThread)){
		goto failure;
    }


success:
#pragma region <returns>

#pragma endregion


    return TRUE;
failure:
#pragma region <cleanup>
    if (b_create_process) {
        TerminateProcess(o_process_info->hProcess, EXIT_FAILURE);

        if (NULL != o_process_info->hThread) {
            CloseHandle(o_process_info->hThread);
        }

        if (NULL != o_process_info->hProcess) {
            CloseHandle(o_process_info->hProcess);
        }
    }
#pragma endregion


    return FALSE;
}


BOOL MemoryCreateProcessExW(IN LPCWSTR ExecutablePath, IN PVOID Image, IN LPWSTR CommandLine, IN STARTUPINFOW* o_startup_info, IN PROCESS_INFORMATION* o_process_info) {
#pragma region <locals>
	BOOL              b_create_process = FALSE,
		              b_read_process_memory = FALSE,
		              b_write_process_memory = FALSE;
    CONTEXT           stCtx;
    PIMAGE_DOS_HEADER lpDOSHeader;
    PIMAGE_NT_HEADERS lpNTHeader;
    LPVOID            lpImageBase = NULL;
#pragma endregion


    /*
     * Check that image is a PE file;
     */
    lpDOSHeader = (PIMAGE_DOS_HEADER)(Image);
#ifdef _WIN64
    lpNTHeader = (PIMAGE_NT_HEADERS)((DWORD64)(Image)+lpDOSHeader->e_lfanew);
#else
    lpNTHeader = (PIMAGE_NT_HEADERS)((DWORD)(Image)+lpDOSHeader->e_lfanew);
#endif // _WIN64

    if (lpNTHeader->Signature != IMAGE_NT_SIGNATURE){
        goto failure;
    }

    /*
     * Create process;
     */
    b_create_process = CreateProcessW(
        ExecutablePath,
        CommandLine,
        NULL,
        NULL,
        FALSE,
        CREATE_SUSPENDED,
        NULL,
        NULL,
        o_startup_info,
        o_process_info
    );

    if (!b_create_process) {
        goto failure;
    }

    /*
     * Allocate memory for the context.
     */
    RtlSecureZeroMemory(&stCtx, sizeof stCtx);

    stCtx.ContextFlags = CONTEXT_FULL;

    if (!GetThreadContext(o_process_info->hThread, &stCtx)){
		goto failure;
    }

#ifndef _WIN64
    b_read_process_memory = ReadProcessMemory(
        o_process_info->hProcess,
        (LPCVOID)(stCtx.Ebx + 8),
        (LPVOID)(&lpImageBase),
        4,
        NULL
    );

    if (!b_read_process_memory) {
        goto failure;
    }
#endif // !_WIN64

    lpImageBase = VirtualAllocEx(
        o_process_info->hProcess,
        (LPVOID)(lpNTHeader->OptionalHeader.ImageBase),
        lpNTHeader->OptionalHeader.SizeOfImage,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE
    );

    if (NULL == lpImageBase) {
        switch (GetLastError()) {
        case ERROR_INVALID_ADDRESS: // Sometimes the start fails due to bad address, so we need to re-try.
            TerminateProcess(o_process_info->hProcess, EXIT_FAILURE);

            CloseHandle(o_process_info->hThread);
            CloseHandle(o_process_info->hProcess);
            
            RtlSecureZeroMemory(o_process_info, sizeof *o_process_info);

            return MemoryCreateProcessExW(ExecutablePath, Image, CommandLine, o_startup_info, o_process_info);
        default:
            goto failure;
        }
    }

    /*
     * Write the image to the process;
     */
    b_write_process_memory = WriteProcessMemory(
        o_process_info->hProcess,
        lpImageBase,
        Image,
        lpNTHeader->OptionalHeader.SizeOfHeaders,
        NULL
    );

    if (!b_write_process_memory) {
		goto failure;
    }

    for (SIZE_T iSection = 0; iSection < lpNTHeader->FileHeader.NumberOfSections; ++iSection){
        PIMAGE_SECTION_HEADER stSectionHeader;
#ifdef _WIN64
        stSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD64)(Image)+lpDOSHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS64) + sizeof(IMAGE_SECTION_HEADER) * iSection);
#else
        stSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)(Image)+lpDOSHeader->e_lfanew + 248 + (iSection * 40));
#endif // _WIN64
		b_write_process_memory = WriteProcessMemory(
			o_process_info->hProcess,
			(LPVOID)((DWORD64)(lpImageBase) + stSectionHeader->VirtualAddress),
			(LPVOID)((DWORD64)(Image)+ stSectionHeader->PointerToRawData),
			stSectionHeader->SizeOfRawData,
			NULL
		);

        if (!b_write_process_memory){
			goto failure;
        }
    }

	b_write_process_memory = WriteProcessMemory(
		o_process_info->hProcess,
#ifdef _WIN64
        (LPVOID)(stCtx.Rdx + sizeof(LPVOID) * 2),
#else
        (LPVOID)(stCtx.Ebx + 8),
#endif // _WIN64
		&lpImageBase,
		sizeof(LPVOID),
		NULL
	);

    if (!b_write_process_memory){
		goto failure;
    }

    /*
     * Move address of entry point to the eax register, set context and resume thread;
     */
#ifdef _WIN64
    stCtx.Rcx = (DWORD64)(lpImageBase)+lpNTHeader->OptionalHeader.AddressOfEntryPoint;
#else
    stCtx.Eax = (DWORD)(lpImageBase)+lpNTHeader->OptionalHeader.AddressOfEntryPoint;
#endif // _WIN64

    if (!SetThreadContext(o_process_info->hThread, &stCtx)){
        goto failure;
    }

    if (!ResumeThread(o_process_info->hThread)){
		goto failure;
    }


success:
#pragma region <returns>

#pragma endregion


    return TRUE;
failure:
#pragma region <cleanup>
    if (b_create_process) {
        TerminateProcess(o_process_info->hProcess, EXIT_FAILURE);

        if (NULL != o_process_info->hThread) {
            CloseHandle(o_process_info->hThread);
        }

        if (NULL != o_process_info->hProcess) {
            CloseHandle(o_process_info->hProcess);
        }
    }
#pragma endregion


    return FALSE;
}

