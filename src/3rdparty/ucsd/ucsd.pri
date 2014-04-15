INCLUDEPATH += $$PWD/nspr-emu

HEADERS += $$files($$PWD/*.h) \
           $$files($$PWD/nspr-emu/*.h) \
           $$files($$PWD/nspr-emu/obsolete.h)

SOURCES += $$files($$PWD/*.cpp)
