TEMPLATE = app

QT     += widgets concurrent
CONFIG += c++11

TARGET = charset-tool \

INCLUDEPATH += src \
               src/3rdparty

HEADERS += \
    src/charset_tool_main_window.h \
    src/input_filter_dialog.h \
    src/editable_list_widget.h \
    src/charset_detector.h

SOURCES += src/main.cpp \
    src/charset_tool_main_window.cpp \
    src/input_filter_dialog.cpp \
    src/editable_list_widget.cpp \
    src/charset_detector.cpp

FORMS += \
    src/charset_tool_main_window.ui \
    src/input_filter_dialog.ui

RESOURCES += \
    charset-tool.qrc


# Mozilla Universal Character Set Detector library
include(src/3rdparty/ucsd/ucsd.pri)

# FougTools
include(src/3rdparty/fougtools/qttools/qttools.pri)


OTHER_FILES += readme.md
