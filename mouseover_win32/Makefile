GCC:=i686-pc-mingw32-gcc.exe -W -Wall -s -O2

.PHONY: all clean

all: GdTextOutHook.dll libGdTextOutHook.a GdTextOutSpy.dll libGdTextOutSpy.a

clean:
	rm -f *.o *.a *.dll

TextOutHook.o: TextOutHook.c
	$(GCC) -DBUILDING_DLL -c $<

HookImportFunction.o: HookImportFunction.c
	$(GCC) -c $<

GetWord.o: GetWord.c
	$(GCC) -c $<

GdTextOutHook.dll libGdTextOutHook.a: TextOutHook.o HookImportFunction.o GetWord.o
	$(GCC) -shared -o GdTextOutHook.dll $^ -lgdi32 -Wl,--out-implib,libGdTextOutHook.a

TextOutSpy.o: TextOutSpy.c
	$(GCC) -DBUILDING_DLL -c $<

ThTypes.o: ThTypes.c
	$(GCC) -c $<

GdTextOutSpy.dll libGdTextOutSpy.a: TextOutSpy.o ThTypes.o
	 $(GCC) -shared -o GdTextOutSpy.dll $^ -lgdi32 -Wl,--out-implib,libGdTextOutSpy.a
