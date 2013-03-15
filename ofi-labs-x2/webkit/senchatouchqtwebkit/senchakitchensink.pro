# Add files and directories to ship with the application 
# by adapting the examples below.
kitchensink.source = html
DEPLOYMENTFOLDERS = kitchensink

# Avoid auto screen rotation
# ORIENTATIONLOCK #
#DEFINES += ORIENTATIONLOCK

# Needs to be defined for Symbian
# NETWORKACCESS #
DEFINES += NETWORKACCESS

# TARGETUID3 #
symbian:TARGET.UID3 = 0xE1234321

# If your application uses the Qt Mobility libraries, uncomment
# the following lines and add the respective components to the 
# MOBILITY variable. 
# CONFIG += mobility
# MOBILITY +=

QT += webkit

SOURCES += main.cpp

# Please do not modify the following two lines. Required for deployment.
include(deployment.pri)
qtcAddDeployment()
