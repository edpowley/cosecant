#-------------------------------------------------
#
# Project created by QtCreator 2018-12-03T02:01:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cosecant_qt
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

QT += xml

INCLUDEPATH += \
    C:/Users/Ed/Documents/prog/portaudio/include \
    zlib-1.2.11 \
    zlib-1.2.11/contrib/minizip

LIBS += -luser32 \
    -lshlwapi \
    -lole32 \
    C:/Users/Ed/Documents/prog/portaudio/build/msvc/x64/Release/portaudio_x64.lib

PRECOMPILED_HEADER = stdafx.h

SOURCES += \
        main.cpp \
        cosecantmainwindow.cpp \
    application.cpp \
    audioio.cpp \
    builtinmachines.cpp \
    callbacks.cpp \
    cosecant_api.cpp \
    cosecantmainwindow.cpp \
    delayline.cpp \
    dlg_about.cpp \
    dlg_machinerename.cpp \
    dlg_settings.cpp \
    dll.cpp \
    dllmachine.cpp \
    eventstream.cpp \
    expanderbox.cpp \
    formatnote.cpp \
    headedview.cpp \
    htmlentity.cpp \
    keyjazz.cpp \
    machine.cpp \
    machinechooserwidget.cpp \
    machinefactory.cpp \
    main.cpp \
    mygraphicsview.cpp \
    parameditor.cpp \
    parameter.cpp \
    prefs.cpp \
    routing.cpp \
    routingeditor.cpp \
    routingeditor_connection.cpp \
    routingeditor_machine.cpp \
    routingeditor_pin.cpp \
    sequence.cpp \
    sequenceeditor.cpp \
    song.cpp \
    song_load.cpp \
    song_save.cpp \
    stdafx.cpp \
    theme.cpp \
    timeunit_convert.cpp \
    uuid.cpp \
    workbuffer.cpp \
    workerthread.cpp \
    workqueue.cpp \
    workunit.cpp \
    zipfilefuncs.cpp \
    zlib-1.2.11/adler32.c \
    zlib-1.2.11/compress.c \
    zlib-1.2.11/crc32.c \
    zlib-1.2.11/deflate.c \
    zlib-1.2.11/gzclose.c \
    zlib-1.2.11/gzlib.c \
    zlib-1.2.11/gzread.c \
    zlib-1.2.11/gzwrite.c \
    zlib-1.2.11/infback.c \
    zlib-1.2.11/inffast.c \
    zlib-1.2.11/inflate.c \
    zlib-1.2.11/inftrees.c \
    zlib-1.2.11/trees.c \
    zlib-1.2.11/uncompr.c \
    zlib-1.2.11/zutil.c \
    zlib-1.2.11/contrib/minizip/ioapi.c \
    zlib-1.2.11/contrib/minizip/iowin32.c \
    zlib-1.2.11/contrib/minizip/mztools.c \
    zlib-1.2.11/contrib/minizip/unzip.c \
    zlib-1.2.11/contrib/minizip/zip.c \
    sqlite/sqlite3.c \
    html_entities/htmlentitymap.cpp

HEADERS += \
        cosecantmainwindow.h \
    application.h \
    audioio.h \
    builtinmachines.h \
    builtinmachines_audioio.h \
    callbacks.h \
    common.h \
    cosecant_api.h \
    cosecant_api_helpers.h \
    cosecantmainwindow.h \
    cursor_raii.h \
    delayline.h \
    dlg_about.h \
    dlg_machinerename.h \
    dlg_settings.h \
    dll.h \
    dllmachine.h \
    error.h \
    eventstream.h \
    expanderbox.h \
    formatnote.h \
    headedview.h \
    hostapi.h \
    htmlentity.h \
    keyjazz.h \
    machine.h \
    machinechooserwidget.h \
    mwtab.h \
    mygraphicsview.h \
    nullable.h \
    object.h \
    parameditor.h \
    parameter.h \
    perfclock.h \
    prefs.h \
    routing.h \
    routingeditor.h \
    sequence.h \
    sequenceeditor.h \
    song.h \
    stdafx.h \
    svnrevision.in.h \
    theme.h \
    timeunit_convert.h \
    undo_command_ids.h \
    utility.h \
    uuid.h \
    version.h \
    workbuffer.h \
    workerthread.h \
    workqueue.h \
    workunit.h \
    xmlutils.h \
    zlib-1.2.11/crc32.h \
    zlib-1.2.11/deflate.h \
    zlib-1.2.11/gzguts.h \
    zlib-1.2.11/inffast.h \
    zlib-1.2.11/inffixed.h \
    zlib-1.2.11/inflate.h \
    zlib-1.2.11/inftrees.h \
    zlib-1.2.11/trees.h \
    zlib-1.2.11/zconf.h \
    zlib-1.2.11/zlib.h \
    zlib-1.2.11/zutil.h \
    zlib-1.2.11/contrib/minizip/crypt.h \
    zlib-1.2.11/contrib/minizip/ioapi.h \
    zlib-1.2.11/contrib/minizip/iowin32.h \
    zlib-1.2.11/contrib/minizip/mztools.h \
    zlib-1.2.11/contrib/minizip/unzip.h \
    zlib-1.2.11/contrib/minizip/zip.h \
    sqlite/sqlite3.h \
    sqlite/sqlite3ext.h

FORMS += \
        cosecantmainwindow.ui \
    about_dlg.ui \
    cosecantmainwindow.ui \
    machine_rename_dlg.ui \
    sequenceeditor_trackheader.ui \
    settings_dlg.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    cosecantmainwindow.qrc
