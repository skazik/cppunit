bTEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    cppunit.cpp

include(deployment.pri)
qtcAddDeployment()

OTHER_FILES += \
    ../builds/build-unitest1/Makefile \
    ../builds/build-unitest1/cppunitResults.xml \
    ../builds/build-unitest1/junitResults.xml \
    cpp2junit.xslt \
    junit_format.xml

LIBS += -lcppunit
#LIBS += /usr/lib/x86_64-linux-gnu/libcppunit.a
SOURCES -= cppunit.cpp

HEADERS += \
    cppunit.h
