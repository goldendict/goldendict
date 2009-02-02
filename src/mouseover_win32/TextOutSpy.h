#ifndef _TextOutSpy_H_
#define _TextOutSpy_H_

#if BUILDING_DLL
# define DLLIMPORT __declspec (dllexport)
#else /* Not BUILDING_DLL */
# define DLLIMPORT __declspec (dllimport)
#endif /* Not BUILDING_DLL */


DLLIMPORT void ActivateTextOutSpying (int Activate);


#endif /* _TextOutSpy_H_ */
