TEMPLATE = app

QT     += widgets concurrent
CONFIG += c++11

TARGET = cassolette
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

INCLUDEPATH += src src/3rdparty

HEADERS += \
    src/abstract_charset_detector.h \
    src/base_file_task.h \
    src/cassolette_main_window.h \
    src/composite_file_task.h \
    src/dir_iteration_task.h \
    src/editable_list_widget.h \
    src/file_charset_detection_task.h \
    src/file_charset_encoding_task.h \
    src/log_widget.h \
    src/mozilla_universal_charset_detector.h \
    src/progress_dialog.h \
    src/select_charset_dialog.h \

SOURCES += \
    src/abstract_charset_detector.cpp \
    src/base_file_task.cpp \
    src/cassolette_main_window.cpp \
    src/composite_file_task.cpp \
    src/dir_iteration_task.cpp \
    src/editable_list_widget.cpp \
    src/file_charset_detection_task.cpp \
    src/file_charset_encoding_task.cpp \
    src/log_widget.cpp \
    src/main.cpp \
    src/mozilla_universal_charset_detector.cpp \
    src/progress_dialog.cpp \
    src/select_charset_dialog.cpp \

FORMS += \
    src/cassolette_main_window.ui \
    src/log_widget.ui \
    src/progress_dialog.ui \
    src/select_charset_dialog.ui \

RESOURCES += cassolette.qrc

win32 {
    HEADERS += src/win_imulti_language_charset_detector.h
    SOURCES += src/win_imulti_language_charset_detector.cpp
    QMAKE_SUBSTITUTES += env_win.bat.in
    LIBS += -lole32 -lShlwapi
}

OTHER_FILES += \
    readme.md \
    deploy/windows/setup.iss \
    $$QMAKE_SUBSTITUTES


# Mozilla Universal Character Set Detector library
include(src/3rdparty/ucsd/ucsd.pri)

# FougTools
include(src/3rdparty/fougtools/qttools/qttools.pri)
