the github action use https://github.com/jurplel/install-qt-action to manage qt version

how to find out the latest qt version and modules

there are several commands 


```
pip install aqt
aqt list-qt linux desktop
aqt list-qt linux desktop --arch 6.2.4
aqt list-qt linux desktop --modules 6.2.4 gcc_64
```