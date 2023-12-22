TEMPLATE = app
TARGET = grapher

QT+=widgets charts 
CONFIG+=debug

SOURCES=main.cpp \
	grapher.cpp \
        device.cpp \
        ttnjson.cpp \
        devicedatasetentry.cpp \
        jsonutils.cpp \
        registeredgraphmodel.cpp \
        newgraphdialog.cpp
        

HEADERS=grapher.h \
	ttnjson.h \
        payload.h \
	devicedataset.h \
        devicedatasetentry.h \
        device.h \
        devicecollection.h \
        registeredgraph.h \
        registeredgraphmodel.h \
        newgraphdialog.h \
        jsonutils.h
