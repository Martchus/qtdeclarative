CONFIG += testcase
TARGET = tst_qdeclarativeqt
SOURCES += tst_qdeclarativeqt.cpp
macx:CONFIG -= app_bundle

testDataFiles.files = data
testDataFiles.path = .
DEPLOYMENT += testDataFiles

CONFIG += parallel_test

QT += core-private gui-private v8-private declarative-private quick-private testlib
