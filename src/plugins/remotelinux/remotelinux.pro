TEMPLATE = lib
TARGET = RemoteLinux

include(../../qtcreatorplugin.pri)
include(remotelinux_dependencies.pri)

HEADERS += \
    remotelinuxplugin.h \
    remotelinux_export.h \
    maemoconstants.h \
    maemorunconfigurationwidget.h \
    maemoruncontrol.h \
    maemorunfactories.h \
    maemosettingspages.h \
    maemotoolchain.h \
    maemopackagecreationstep.h \
    maemopackagecreationfactory.h \
    maemopackagecreationwidget.h \
    maemoqemumanager.h \
    maemodeploystepfactory.h \
    maemoglobal.h \
    maemosshrunner.h \
    maemodebugsupport.h \
    maemoremotemountsmodel.h \
    maemomountspecification.h \
    maemoremotemounter.h \
    maemoprofilesupdatedialog.h \
    maemopublishingwizardfactories.h \
    maemopublishingbuildsettingspagefremantlefree.h \
    maemopublishingfileselectiondialog.h \
    maemopublishedprojectmodel.h \
    maemopublishinguploadsettingspagefremantlefree.h \
    maemopublishingwizardfremantlefree.h \
    maemopublishingresultpagefremantlefree.h \
    maemopublisherfremantlefree.h \
    maemoqemuruntime.h \
    maemoqemuruntimeparser.h \
    maemoqemusettingswidget.h \
    maemoqemusettings.h \
    qt4maemotargetfactory.h \
    qt4maemotarget.h \
    qt4maemodeployconfiguration.h \
    maemodeviceconfigwizard.h \
    maemodeployconfigurationwidget.h \
    maemoinstalltosysrootstep.h \
    maemodeploymentmounter.h \
    maemopackageinstaller.h \
    maemoremotecopyfacility.h \
    maemoqtversionfactory.h \
    maemoqtversion.h \
    linuxdeviceconfiguration.h \
    linuxdeviceconfigurations.h \
    remotelinuxrunconfiguration.h \
    linuxdevicefactoryselectiondialog.h \
    publickeydeploymentdialog.h \
    genericlinuxdeviceconfigurationwizard.h \
    genericlinuxdeviceconfigurationfactory.h \
    maddedeviceconfigurationfactory.h \
    maemorunconfiguration.h \
    remotelinuxrunconfigurationwidget.h \
    remotelinuxrunconfigurationfactory.h \
    remotelinuxapplicationrunner.h \
    remotelinuxruncontrol.h \
    remotelinuxruncontrolfactory.h \
    remotelinuxdebugsupport.h \
    genericlinuxdeviceconfigurationwizardpages.h \
    portlist.h \
    deployablefile.h \
    deployablefilesperprofile.h \
    deploymentinfo.h \
    abstractremotelinuxdeploystep.h \
    genericdirectuploadstep.h \
    maddeuploadandinstallpackagesteps.h \
    uploadandinstalltarpackagestep.h \
    maemodeploybymountsteps.h \
    abstractremotelinuxdeployservice.h \
    abstractuploadandinstallpackageservice.h \
    genericdirectuploadservice.h \
    remotelinuxdeployconfiguration.h \
    remotelinuxdeployconfigurationfactory.h \
    genericremotelinuxdeploystepfactory.h \
    abstractpackagingstep.h \
    tarpackagecreationstep.h \
    remotelinuxpackageinstaller.h \
    packageuploader.h \
    linuxdevicetester.h \
    maddedevicetester.h \
    remotelinux_constants.h \
    linuxdevicetestdialog.h \
    remotelinuxprocesslist.h \
    madderemoteprocesslist.h \
    remotelinuxprocessesdialog.h \
    remotelinuxdeploystepwidget.h \
    linuxdeviceconfigurationssettingswidget.h \
    remotelinuxenvironmentreader.h \
    sshkeydeployer.h \
    typespecificdeviceconfigurationlistmodel.h \
    sshkeycreationdialog.h \
    remotelinuxusedportsgatherer.h

SOURCES += \
    remotelinuxplugin.cpp \
    maemorunconfigurationwidget.cpp \
    maemoruncontrol.cpp \
    maemorunfactories.cpp \
    maemosettingspages.cpp \
    maemotoolchain.cpp \
    maemopackagecreationstep.cpp \
    maemopackagecreationfactory.cpp \
    maemopackagecreationwidget.cpp \
    maemoqemumanager.cpp \
    maemodeploystepfactory.cpp \
    maemoglobal.cpp \
    maemosshrunner.cpp \
    maemodebugsupport.cpp \
    maemoremotemountsmodel.cpp \
    maemomountspecification.cpp \
    maemoremotemounter.cpp \
    maemoprofilesupdatedialog.cpp \
    maemopublishingwizardfactories.cpp \
    maemopublishingbuildsettingspagefremantlefree.cpp \
    maemopublishingfileselectiondialog.cpp \
    maemopublishedprojectmodel.cpp \
    maemopublishinguploadsettingspagefremantlefree.cpp \
    maemopublishingwizardfremantlefree.cpp \
    maemopublishingresultpagefremantlefree.cpp \
    maemopublisherfremantlefree.cpp \
    maemoqemuruntimeparser.cpp \
    maemoqemusettingswidget.cpp \
    maemoqemusettings.cpp \
    qt4maemotargetfactory.cpp \
    qt4maemotarget.cpp \
    qt4maemodeployconfiguration.cpp \
    maemodeviceconfigwizard.cpp \
    maemodeployconfigurationwidget.cpp \
    maemoinstalltosysrootstep.cpp \
    maemodeploymentmounter.cpp \
    maemopackageinstaller.cpp \
    maemoremotecopyfacility.cpp \
    maemoqtversionfactory.cpp \
    maemoqtversion.cpp \
    linuxdeviceconfiguration.cpp \
    linuxdeviceconfigurations.cpp \
    remotelinuxrunconfiguration.cpp \
    linuxdevicefactoryselectiondialog.cpp \
    publickeydeploymentdialog.cpp \
    genericlinuxdeviceconfigurationwizard.cpp \
    genericlinuxdeviceconfigurationfactory.cpp \
    maddedeviceconfigurationfactory.cpp \
    maemorunconfiguration.cpp \
    remotelinuxrunconfigurationwidget.cpp \
    remotelinuxrunconfigurationfactory.cpp \
    remotelinuxapplicationrunner.cpp \
    remotelinuxruncontrol.cpp \
    remotelinuxruncontrolfactory.cpp \
    remotelinuxdebugsupport.cpp \
    genericlinuxdeviceconfigurationwizardpages.cpp \
    portlist.cpp \
    deployablefilesperprofile.cpp \
    deploymentinfo.cpp \
    abstractremotelinuxdeploystep.cpp \
    genericdirectuploadstep.cpp \
    maddeuploadandinstallpackagesteps.cpp \
    uploadandinstalltarpackagestep.cpp \
    maemodeploybymountsteps.cpp \
    abstractremotelinuxdeployservice.cpp \
    abstractuploadandinstallpackageservice.cpp \
    genericdirectuploadservice.cpp \
    remotelinuxdeployconfiguration.cpp \
    remotelinuxdeployconfigurationfactory.cpp \
    genericremotelinuxdeploystepfactory.cpp \
    abstractpackagingstep.cpp \
    tarpackagecreationstep.cpp \
    remotelinuxpackageinstaller.cpp \
    packageuploader.cpp \
    linuxdevicetester.cpp \
    maddedevicetester.cpp \
    linuxdevicetestdialog.cpp \
    remotelinuxprocesslist.cpp \
    madderemoteprocesslist.cpp \
    remotelinuxprocessesdialog.cpp \
    remotelinuxdeploystepwidget.cpp \
    linuxdeviceconfigurationssettingswidget.cpp \
    remotelinuxenvironmentreader.cpp \
    sshkeydeployer.cpp \
    typespecificdeviceconfigurationlistmodel.cpp \
    sshkeycreationdialog.cpp \
    remotelinuxusedportsgatherer.cpp

FORMS += \
    maemopackagecreationwidget.ui \
    maemoprofilesupdatedialog.ui \
    maemopublishingbuildsettingspagefremantlefree.ui \
    maemopublishingfileselectiondialog.ui \
    maemopublishinguploadsettingspagefremantlefree.ui \
    maemopublishingresultpagefremantlefree.ui \
    maemoqemusettingswidget.ui \
    maemodeviceconfigwizardstartpage.ui \
    maemodeviceconfigwizardpreviouskeysetupcheckpage.ui \
    maemodeviceconfigwizardreusekeyscheckpage.ui \
    maemodeviceconfigwizardkeycreationpage.ui \
    maemodeviceconfigwizardkeydeploymentpage.ui \
    maemodeployconfigurationwidget.ui \
    linuxdevicefactoryselectiondialog.ui \
    genericlinuxdeviceconfigurationwizardsetuppage.ui \
    linuxdevicetestdialog.ui \
    remotelinuxprocessesdialog.ui \
    linuxdeviceconfigurationssettingswidget.ui \
    sshkeycreationdialog.ui

RESOURCES += qt-maemo.qrc
DEFINES += QT_NO_CAST_TO_ASCII
DEFINES += REMOTELINUX_LIBRARY
