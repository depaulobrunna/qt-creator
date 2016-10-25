include(../../qtcreatorplugin.pri)
QT += gui-private network printsupport serialport
HEADERS += openmvplugin.h \
           openmvpluginserialport.h \
           openmvpluginio.h \
           openmvpluginfb.h \
           histogram/openmvpluginhistogram.h \
           qcustomplot/qcustomplot.h
SOURCES += openmvplugin.cpp \
           openmvpluginserialport.cpp \
           openmvpluginio.cpp \
           openmvpluginfb.cpp \
           histogram/openmvpluginhistogram.cpp \
           histogram/rgb2rgb_tab.c \
           histogram/lab_tab.c \
           histogram/yuv_tab.c  \
           qcustomplot/qcustomplot.cpp
FORMS += histogram/openmvpluginhistogram.ui
RESOURCES += openmv.qrc
