TEMPLATE = subdirs
CONFIG += console

#Library targets and depdendencies
SUBDIRS = cppCORE\
        cppXML\
        cppGUI\
        geGUI

cppXML.depends = cppCORE
cppGUI.depends = cppCORE
geGUI.depends = cppCORE cppXML cppGUI
