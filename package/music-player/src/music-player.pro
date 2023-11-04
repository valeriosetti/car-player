QT += quick

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        custom_list_model.cpp \
        gstmanager.cpp \
        main.cpp \
        path_mngr.cpp \
        spectrumgraph.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

HEADERS += \
    custom_list_model.h \
    gstmanager.h \
    main.h \
    path_mngr.h \
    spectrumgraph.h

SYSROOT_DIR = $$(HOST_DIR)/aarch64-buildroot-linux-gnu/sysroot
message(SYSROOT_DIR --> $$SYSROOT_DIR)

QMAKE_CXXFLAGS += -pthread
QMAKE_CXXFLAGS += -I$$SYSROOT_DIR/usr/include/gstreamer-1.0/
QMAKE_CXXFLAGS += -I$$SYSROOT_DIR/usr/include/glib-2.0
QMAKE_CXXFLAGS += -I$$SYSROOT_DIR/usr/lib/glib-2.0/include
QMAKE_CXXFLAGS += -I$$SYSROOT_DIR/usr/lib/x86_64-linux-gnu/glib-2.0/include

LIBS +=  -lgstreamer-1.0 -lgobject-2.0 -lglib-2.0
