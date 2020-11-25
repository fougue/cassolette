Introduction
------------

`Cassolette` is a free and multi-platform utility software to help analysing and converting the
encoding of text files.

To detect file encoding, Cassolette internally uses the Mozilla's Universal Character Set Detector
library (UCSD for short).

Code source of Mozilla's UCSD library was taken from :
  https://github.com/batterseapower/libcharsetdetect

How To Build
------------

`Cassolette` is developed with Qt5 / C++11 so a recent C++ compiler is required.
If you don't have Qt yet, pre-compiled binaries can be installed with the [Qt installer](https://www.qt.io/download-qt-installer)

Then type these commands from the root directory of cassolette :

```bash
    qmake
    (n)make
```

License
-------

`Cassolette` is available under CeCILL-B which is a BSD-like license. For more information, please
see http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html

Copyright (c) 2015, Fougue Ltd.
