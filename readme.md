Introduction
------------

charset-tool is a free and multi-platform utility software to help analysing and converting the
encoding of text files.

To detect file encoding, charset-tool internally uses the Mozilla's Universal Character Set Detector
library (UCSD for short).

Code source of Mozilla's UCSD library was taken from :
  https://github.com/batterseapower/libcharsetdetect

How To Build
------------

charset-tool is developed with Qt5 / C++11 so a recent C++ compiler is required.
If you don't have Qt yet, pre-compiled binaries can be downloaded at http://qt-project.org/downloads

Once Qt built or installed with a pre-compiled package, then type these commands from the root
directory of charset-tool :

    qmake
    (n)make

License
-------

charset-tool is available under CeCILL-B which is a BSD-like license. For more information, please
see http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html

Copyright (c) 2014, FougSys Ltd.
