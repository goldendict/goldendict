#ifndef __GetWordByIAccEx_H_DEFINED_
#define __GetWordByIAccEx_H_DEFINED_

#ifdef __cplusplus
extern "C" {
#endif

typedef BOOL (*GetPhysicalCursorPosFunc)(LPPOINT);
extern GetPhysicalCursorPosFunc getPhysicalCursorPosFunc;

BOOL FindGetPhysicalCursorPos();

BOOL getWordByAccEx( POINT pt );

#ifdef __cplusplus
}
#endif

#endif
