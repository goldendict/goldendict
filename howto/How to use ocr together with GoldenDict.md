# Current Situation
GoldenDict did offer a functionality to translate the word under cursor(when scan popup been enabled) on Windows.
the technique used there is old and can not work crossplatform .

with the help of another great tool [Capture2Text](https://sourceforge.net/projects/capture2text/) ,GoldenDict can work with ocr seamlessly.

# Note
Capture2Text has offered Windows precompiled executable files.I have ported it to Linux https://github.com/xiaoyifang/Capture2Text

Thanks to:
- [Capture2Text Linux Port](https://github.com/GSam/Capture2Text )
- [another on nixos](https://github.com/sikmir/nur-packages/blob/7c876e3fb20160781207a8f652fb052647e6da0d/pkgs/misc/capture2text/default.nix) from [sikmir](https://github.com/goldendict/goldendict/issues/1445#issuecomment-1022972220)

# Download release or compile by yourself
https://github.com/xiaoyifang/Capture2Text/releases
# Configuration on Windows
after installation of both software.
configure the Capture2Text ,see screenshots below.
## configure external `Call Executable` path   (Windows for example)
`path\GoldenDict.exe "${capture}"`

![image](https://user-images.githubusercontent.com/105986/151507994-97ab732d-686a-47b1-b950-3b2db076ef4c.png)

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

![image](https://user-images.githubusercontent.com/105986/151489807-71231884-75bf-45e7-9bfa-b5242be1b189.png)



## use Capture2Text on Linxu

### download and build the capture2text
https://github.com/xiaoyifang/Capture2Text

![2022-01-30 15-54-35屏幕截图](https://user-images.githubusercontent.com/105986/151691526-f28cc053-f6e0-4099-b677-f7a4657aa9fc.png)

### settings

![2022-01-30 15-54-35屏幕截图1](https://user-images.githubusercontent.com/105986/151691583-eda3e059-a77f-4476-a5a5-18d34463005e.png)

![image](https://user-images.githubusercontent.com/105986/151694194-7f0048fc-5649-46b3-940f-d4d5d10968b7.png)

### start capture

`Ctrl+Shift+Q`

![image](https://user-images.githubusercontent.com/105986/151691692-955caf26-e828-4ffe-a630-b17b66b8a955.png)


### end capture
press `Ctrl+Shift+Q` again.





