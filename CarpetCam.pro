#-------------------------------------------------
#
# Project created by QtCreator 2017-11-29T19:59:56
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CarpetCam
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    globalvariable.cpp \
    quantification.cpp \
    cluster.cpp \
    spliter.cpp \
    tracker.cpp \
    curvefit.cpp \
    showpic.cpp \
    bezieroffset.cpp \
    fillcolors.cpp \
    anyanglefill.cpp \
    yfillcolors.cpp

HEADERS  += mainwindow.h \
    globalvariable.h \
    datatype.h \
    quantification.h \
    cluster.h \
    spliter.h \
    tracker.h \
    curvefit.h \
    showpic.h \
    entitytype.h \
    bezieroffset.h \
    fillcolors.h \
    anyanglefill.h \
    yfillcolors.h

FORMS    += mainwindow.ui \
    showpic.ui


INCLUDEPATH += D:/opencv/include\
                            D:/opencv/include/opencv\
                            D:/opencv/include/opencv2



LIBS += -LD:/opencv/bin -llibopencv_calib3d320
LIBS += -LD:/opencv/bin -llibopencv_core320
LIBS += -LD:/opencv/bin -llibopencv_features2d320
LIBS += -LD:/opencv/bin -llibopencv_flann320
LIBS += -LD:/opencv/bin -llibopencv_highgui320
LIBS += -LD:/opencv/bin -llibopencv_imgcodecs320
LIBS += -LD:/opencv/bin -llibopencv_imgproc320
LIBS += -LD:/opencv/bin -llibopencv_ml320
LIBS += -LD:/opencv/bin -llibopencv_objdetect320
LIBS += -LD:/opencv/bin -llibopencv_photo320
LIBS += -LD:/opencv/bin -llibopencv_shape320
LIBS += -LD:/opencv/bin -llibopencv_stitching320
LIBS += -LD:/opencv/bin -llibopencv_superres320
LIBS += -LD:/opencv/bin -llibopencv_video320
LIBS += -LD:/opencv/bin -llibopencv_videoio320
LIBS += -LD:/opencv/bin -llibopencv_videostab320

#LIBS += -LD:/opencv/lib -llibopencv_calib3d320
#LIBS += -LD:/opencv/lib -llibopencv_core320
#LIBS += -LD:/opencv/lib -llibopencv_features2d320
#LIBS += -LD:/opencv/lib -llibopencv_flann320
#LIBS += -LD:/opencv/lib -llibopencv_highgui320
#LIBS += -LD:/opencv/lib -llibopencv_imgcodecs320
#LIBS += -LD:/opencv/lib -llibopencv_imgproc320
#LIBS += -LD:/opencv/lib -llibopencv_ml320
#LIBS += -LD:/opencv/lib -llibopencv_objdetect320
#LIBS += -LD:/opencv/lib -llibopencv_photo320
#LIBS += -LD:/opencv/lib -llibopencv_shape320
#LIBS += -LD:/opencv/lib -llibopencv_stitching320
#LIBS += -LD:/opencv/lib -llibopencv_superres320
#LIBS += -LD:/opencv/lib -llibopencv_video320
#LIBS += -LD:/opencv/lib -llibopencv_videoio320
#LIBS += -LD:/opencv/lib -llibopencv_videostab320
