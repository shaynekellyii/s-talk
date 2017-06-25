#ifndef _MAIN_H_
#define _MAIN_H_

/***************************************************************
 * Function Prototypes                                         *
 ***************************************************************/
void InitSocket();
void *AcceptKeyboardInput(void *ptr);
void *PrintMessages(void *ptr);
void *SendToSocket(void *ptr);
void *ReceiveFromSocket(void *ptr);
void die();
void FreeItem();
void CheckIfThreadsReady();
void PrintReadyMessage();

#endif /* _MAIN_H_ */