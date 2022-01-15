# prerequisite:
+ [VS2019 ](https://visualstudio.microsoft.com/zh-hans/vs/)
- QT
- QT tool for VS(installed from VS extension)


# steps: 

- configure qt path
toolbar Extensions=>Qt VS Tools=>Options=>versions.
add qt installation folder

- Open pro project file directly throughttoolbar Extensions=>Qt VS Tools=>Open Qt Project file(.pro)


ref: 
[1](https://blog.csdn.net/qq_43493715/article/details/109839046)

# build


# debug
after successfully build,in the target folder(where goldendict.exe resides) ,run windeployqt which copy all the necessary files to this folder.
and copy other missing dlls to this folder. you can click the exe to verify the application can run .

after alll this ,you can debug the application normally.
