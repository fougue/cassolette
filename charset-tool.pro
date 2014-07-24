TEMPLATE = app

QT     += widgets concurrent
CONFIG += c++11

TARGET = charset-tool
CONFIG(debug, debug|release) {
  TARGET = $$join(TARGET, , , _d)
} else {
  CONFIG -= console
}

contains(QT_ARCH, i386) {
  APP_TARGET_ARCH = x86_32
} else {
  APP_TARGET_ARCH = $$QT_ARCH
}

INCLUDEPATH += src \
               src/3rdparty

HEADERS += \
    src/charset_tool_main_window.h \
    src/select_charset_dialog.h \
    src/base_file_task.h \
    src/progress_dialog.h \
    src/log_widget.h \
    src/composite_file_task.h \
    src/editable_list_widget.h \
    src/file_charset_detection_task.h \
    src/file_charset_encoding_task.h \
    src/dir_iteration_task.h

SOURCES += \
    src/main.cpp \
    src/charset_tool_main_window.cpp \
    src/select_charset_dialog.cpp \
    src/base_file_task.cpp \
    src/progress_dialog.cpp \
    src/log_widget.cpp \
    src/composite_file_task.cpp \
    src/editable_list_widget.cpp \
    src/file_charset_detection_task.cpp \
    src/file_charset_encoding_task.cpp \
    src/dir_iteration_task.cpp

FORMS += \
    src/charset_tool_main_window.ui \
    src/select_charset_dialog.ui \
    src/progress_dialog.ui \
    src/log_widget.ui

RESOURCES += charset-tool.qrc

win32:QMAKE_SUBSTITUTES += env_win.bat.in

OTHER_FILES += readme.md \
               deploy/windows/setup.iss \
               $$QMAKE_SUBSTITUTES


# Mozilla Universal Character Set Detector library
include(src/3rdparty/ucsd/ucsd.pri)

# FougTools
include(src/3rdparty/fougtools/qttools/qttools.pri)
