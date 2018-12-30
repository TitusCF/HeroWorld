#include <Qt>
#include <QApplication>
#include <QCoreApplication>

#include <CREMainWindow.h>
#include "CRESettings.h"

#include "CREPixmap.h"

#include "CREFilterDefinition.h"
#include "CREFilterDefinitionManager.h"
#include "CREReportDefinition.h"
#include "CREReportDefinitionManager.h"


int main(int argc, char **argv) {
    QCoreApplication::setOrganizationName("The Legendary Team of Ailesse");
    QCoreApplication::setApplicationName("CRE");
    QApplication app(argc, argv);

    qRegisterMetaTypeStreamOperators<CREFilterDefinition>("CREFilterDefinition");
    qRegisterMetaTypeStreamOperators<CREFilterDefinitionManager>("CREFilterDefinitionManager");
    qRegisterMetaTypeStreamOperators<CREReportDefinition>("CREReportDefinition");
    qRegisterMetaTypeStreamOperators<CREReportDefinitionManager>("CREReportDefinitionManager");

    CREPixmap::init();

    CRESettings settings;
    if (!settings.ensureOptions())
        return -1;

    CREMainWindow win;
    win.show();

    return app.exec();
}
