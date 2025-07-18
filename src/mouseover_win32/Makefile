# Build x86 target by mingw 4.8
GCC:=gcc.exe -m32 -W -Wall -s -O2 -fno-omit-frame-pointer -static
GCC64:=gcc.exe -m64 -W -Wall -s -O2 -fno-omit-frame-pointer -static

.PHONY: x86 x64 clean

x86: GdTextOutHook.dll libGdTextOutHook.a GdTextOutSpy.dll libGdTextOutSpy.a x86helper.exe

x64: x64helper.exe GdTextOutHook64.dll libGdTextOutHook64.a GdTextOutSpy64.dll libGdTextOutSpy64.a

clean:
	rm -f *.o *.a *.dll *.exe

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

IAccEx.o: IAccEx.c
	$(GCC) -c $<

GetWordByIAccEx.o: GetWordByIAccEx.c
	$(GCC) -c $<

guids.o: guids.c
	$(GCC) -c $<

GdTextOutSpy.dll libGdTextOutSpy.a: TextOutSpy.o ThTypes.o IAccEx.o guids.o GetWordByIAccEx.o
	$(GCC) -shared -o GdTextOutSpy.dll $^ -lgdi32 -luuid -loleacc -loleaut32 -Wl,--out-implib,libGdTextOutSpy.a

x64helper.exe: x64hooks64.o ThTypes64.o
	$(GCC64) -Wl,--subsystem,windows -mwindows -o $@ $^

x64hooks64.o: x64hooks.c
	$(GCC64) -o $@ -c $<

TextOutHook64.o: TextOutHook.c
	$(GCC64) -o $@ -DBUILDING_DLL -c $<

HookImportFunction64.o: HookImportFunction.c
	$(GCC64) -o $@ -c $<

GetWord64.o: GetWord.c
	$(GCC64) -o $@ -c $<

GdTextOutHook64.dll libGdTextOutHook64.a: TextOutHook64.o HookImportFunction64.o GetWord64.o
	$(GCC64) -shared -o GdTextOutHook64.dll $^ -lgdi32 -Wl,--out-implib,libGdTextOutHook64.a

TextOutSpy64.o: TextOutSpy.c
	$(GCC64) -o $@ -DBUILDING_DLL -c $<

ThTypes64.o: ThTypes.c
	$(GCC64) -o $@ -c $<

IAccEx64.o: IAccEx.c
	$(GCC64) -o $@ -c $<

GetWordByIAccEx64.o: GetWordByIAccEx.c
	$(GCC64) -o $@ -c $<

guids64.o: guids.c
	$(GCC64) -o $@ -c $<

GdTextOutSpy64.dll libGdTextOutSpy64.a: TextOutSpy64.o ThTypes64.o IAccEx64.o guids64.o GetWordByIAccEx64.o
	$(GCC64) -shared -o GdTextOutSpy64.dll $^ -lgdi32 -luuid -loleacc -loleaut32 -Wl,--out-implib,libGdTextOutSpy64.a

x64hooks86.o: x64hooks.c
	$(GCC) -o $@ -c $<

x86helper.exe: x64hooks86.o ThTypes.o
	$(GCC) -Wl,--subsystem,windows -mwindows -o $@ $^
