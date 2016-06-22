/*                                         *\

   ***************************************
   **                                   **
   **    HELLCAT's Demo #1              **
   **    The crappy side of life ;-)    **
   **                                   **
   ***************************************

            |\      _,,,---,,_
     ZZZzz /,`.-'`'    -.  ;-;;,_
          |,4-  ) )-,_. ,\ (  `'-'
         '---''(_/--'  `-'\_)

\*                                         */

// INCLUDES
#include <pspkernel.h> 
#include <pspdebug.h> 
#include <pspdisplay.h>
#include <stdlib.h> 
#include <string.h>
#include "pspcore.h"
#include "common.h"
#include "part0.h"


// Main defs
PSP_MODULE_INFO("HCDEMO1", 0, 1, 0); 
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);



// main thread
int main(void)
{
	// init the debug output "console" for messages before the actual start of the real stuff :-)
	pspDebugScreenInit();
	/*pspDebugScreenPrintf("HELLCATs Crappy Demo 1\ninit....\n------------------------------------------------\n\n");
	pspDebugScreenPrintf("            |\\      _,,,---,,_\n     ZZZzz /,`.-'`'    -.  ;-;;,_\n          |,4-  ) )-,_. ,\\ (  `'-'\n         '---''(_/--'  `-'\\_)\n");
    pspDebugScreenPrintf("------------------------------------------------\n\n");
*/
    // Install the Exit-Game Callback
	SetupExitCallback();

	// init the random number generator for future use
	RandomInit();

	hcWait(3);

	// ASCII intro
	hcd1RunPart0();

	// THE END
	while (1)
	{
		sceDisplayWaitVblankStart();
	}
	return 0;
}
