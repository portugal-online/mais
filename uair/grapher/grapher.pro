TEMPLATE = app
TARGET = grapher

QT+=widgets charts 
CONFIG+=debug

SOURCES=main.cpp grapher.cpp ttnjson.cpp datasetentry.cpp jsonutils.cpp

HEADERS=grapher.h ttnjson.h payload.h \
	dataset.h datasetentry.h \
        device.h devicecollection.h \
        jsonutils.h
