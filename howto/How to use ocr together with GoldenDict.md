# Current Situation
GoldenDict did offer a functionality to translate the word under cursor(when scan popup been enabled) on Windows.
the technique used there is old and can not work crossplatform .

with the help of another great tool [Capture2Text](https://sourceforge.net/projects/capture2text/) ,GoldenDict can work with ocr seamlessly.

# Note
Capture2Text has offered Windows precompiled executable files.I think it would be easier and possible to make it work on Linux as some afforts have tried.
- [Capture2Text Linux Port](https://github.com/GSam/Capture2Text )
- [another on nixos](https://github.com/sikmir/nur-packages/blob/7c876e3fb20160781207a8f652fb052647e6da0d/pkgs/misc/capture2text/default.nix) from [sikmir](https://github.com/goldendict/goldendict/issues/1445#issuecomment-1022972220)

# Configuration 
after installation of both software.
configure the Capture2Text ,see screenshot following
## configure external `Call Executable` path   (Windows for example)
`path\GoldenDict.exe ${capture}`

![image](https://user-images.githubusercontent.com/105986/151481166-806c4866-601d-4223-b18c-5bc5d078607e.png)

## configure hotkey
I only left three of them .
![image](https://user-images.githubusercontent.com/105986/151481239-16cbb733-746c-425d-bc6c-2bb5e5a158c5.png)

## configure `First word only`
without this configuration ,it will capture the last word of 
![image](https://user-images.githubusercontent.com/105986/151481312-4e9bc457-6667-4e80-95bd-6f2ad58c37e1.png)


## Use it
place the cursor on the word (can be on the  image ),press 
- Win+W to capture the first word.

![image](https://user-images.githubusercontent.com/105986/151481735-6c1c7fc1-715f-4f5c-a98c-7452099b9709.png)

- Win+Q select rectangle.

![image](https://user-images.githubusercontent.com/105986/151489148-6fb09787-8d27-4c55-92bb-b385e23ed859.png)

result:

![image](https://user-images.githubusercontent.com/105986/151489564-a70ce1f5-0101-4bb2-bc6c-020553d62cf1.png)







