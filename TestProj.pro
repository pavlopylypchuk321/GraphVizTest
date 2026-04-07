QT += core gui widgets svg svgwidgets

CONFIG += c++17

SOURCES += \
    main.cpp

# GraphViz via MacPorts (pkg-config also works if PKG_CONFIG_PATH includes /opt/local/lib/pkgconfig)
macx {
    INCLUDEPATH += /opt/local/include
    LIBS += -L/opt/local/lib -lgvc -lcgraph -lcdt
    QMAKE_LFLAGS += -Wl,-rpath,/opt/local/lib
}

# Alternative: uncomment if you prefer pkg-config and have it on PATH
# CONFIG += link_pkgconfig
# PKGCONFIG += libgvc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
