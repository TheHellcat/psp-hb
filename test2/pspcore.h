/*                      *\

  **********************
  **                  **
  **  PSP Core Stuff  **
  **                  **
  **********************

  === HEADER INCLUDE ===

\*                       */

//ifdef __cplusplus
//extern "C" {
//#endif

int Exit_Callback(int arg1, int arg2, void *common);
int ExitCallbackThread(SceSize args, void *argp);
int SetupExitCallback(void);

//ifdef __cplusplus
//}
//#endif
