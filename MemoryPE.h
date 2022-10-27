#
#pragma region <imports>
#pragma region "export header imports"

#pragma endregion
#
#pragma region "platform-dependent imports"
#include <Windows.h>
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
/// <summary>
/// Creates a process from memory using the current 
/// executable as base to map @Image to.
/// 
/// It should be noted that the image provided (@Image) 
/// must match in architecture that of the calling 
/// process (= if calling process is 32-bit, the image 
/// must also be 32-bit)
/// </summary>
/// <param name="Image">
/// Image of Portable Executable to call
/// </param>
/// <param name="CommandLine">
/// [optional] Commandline string to pass onto created 
/// process
/// </param>
/// <param name="o_startup_info">
/// Pointer to startup info to create process with
/// </param>
/// <param name="o_process_info">
/// Pointer to process info to receive process data
/// </param>
/// <returns>
/// TRUE if succeeded, FALSE otherwise;
/// GetLastError() may be called but so far is 
/// unreliable.
/// </returns>
BOOL MemoryCreateProcessA(
	IN PVOID                Image, 
	IN LPSTR                CommandLine,
	IN STARTUPINFOA*        o_startup_info,
	IN PROCESS_INFORMATION* o_process_info 
);


/// <summary>
/// Creates a process from memory using the current 
/// executable as base to map @Image to.
/// 
/// It should be noted that the image provided (@Image) 
/// must match in architecture that of the calling 
/// process (= if calling process is 32-bit, the image 
/// must also be 32-bit)
/// </summary>
/// <param name="Image">
/// Image of Portable Executable to call
/// </param>
/// <param name="CommandLine">
/// [optional] Commandline string to pass onto created 
/// process
/// </param>
/// <param name="o_startup_info">
/// Pointer to startup info to create process with
/// </param>
/// <param name="o_process_info">
/// Pointer to process info to receive process data
/// </param>
/// <returns>
/// TRUE if succeeded, FALSE otherwise;
/// GetLastError() may be called but so far is 
/// unreliable.
/// </returns>
BOOL MemoryCreateProcessW(
	IN PVOID                Image, 
	IN LPWSTR               CommandLine,
	IN STARTUPINFOW*        o_startup_info,
	IN PROCESS_INFORMATION* o_process_info 
);


/// <summary>
/// Creates a process from memory using the executable 
/// located at the provided @ExecutablePath as base to
/// map @Image into.
/// 
/// It should be noted that the image provided (@Image)
/// must match in architecture that of executable 
/// located at @ExecutablePath (= if executable is 
/// 32-bit, the image must also be 32-bit)
/// </summary>
/// <param name="ExecutablePath">
/// Path to Portable Executable to use as base for image 
/// execution
/// </param>
/// <param name="Image">
/// Image of Portable Executable to call
/// </param>
/// <param name="CommandLine">
/// [optional] Commandline string to pass onto created 
/// process
/// </param>
/// <param name="o_startup_info">
/// Pointer to startup info to create process with
/// </param>
/// <param name="o_process_info">
/// Pointer to process info to receive process data
/// </param>
/// <returns>
/// TRUE if succeeded, FALSE otherwise;
/// GetLastError() may be called but so far is 
/// unreliable.
/// </returns>
BOOL MemoryCreateProcessExA(
	IN LPCSTR               ExecutablePath,
	IN PVOID                Image,
	IN LPSTR                CommandLine,
	IN STARTUPINFOA*        o_startup_info,
	IN PROCESS_INFORMATION* o_process_info 
);


/// <summary>
/// Creates a process from memory using the executable 
/// located at the provided @ExecutablePath as base to
/// map @Image into.
/// 
/// It should be noted that the image provided (@Image)
/// must match in architecture that of executable 
/// located at @ExecutablePath (= if executable is 
/// 32-bit, the image must also be 32-bit)
/// </summary>
/// <param name="ExecutablePath">
/// Path to Portable Executable to use as base for image 
/// execution
/// </param>
/// <param name="Image">
/// Image of Portable Executable to call
/// </param>
/// <param name="CommandLine">
/// [optional] Commandline string to pass onto created 
/// process
/// </param>
/// <param name="o_startup_info">
/// Pointer to startup info to create process with
/// </param>
/// <param name="o_process_info">
/// Pointer to process info to receive process data
/// </param>
/// <returns>
/// TRUE if succeeded, FALSE otherwise;
/// GetLastError() may be called but so far is 
/// unreliable.
/// </returns>
BOOL MemoryCreateProcessExW(
	IN LPCWSTR              ExecutablePath,
	IN PVOID                Image,
	IN LPWSTR               CommandLine,
	IN STARTUPINFOW*        o_startup_info,
	IN PROCESS_INFORMATION* o_process_info 
);


#if defined(_MBCS) && _MBCS == 1
#define MemoryCreateProcess		MemoryCreateProcessA
#define MemoryCreateProcessEx	MemoryCreateProcessExA
#elif defined(_UNICODE) && _UNICODE == 1
#define MemoryCreateProcess		MemoryCreateProcessW
#define MemoryCreateProcessEx	MemoryCreateProcessExW
#endif


