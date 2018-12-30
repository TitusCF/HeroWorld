#include "CREMapPanel.h"
#include "CREMapInformation.h"
#include "CREMainWindow.h"

CREMapPanel::CREMapPanel()
{
    QGridLayout* layout = new QGridLayout(this);

    layout->addWidget(new QLabel(tr("Name:"), this), 0, 0);
    myName = new QLabel();
    layout->addWidget(myName, 0, 1);

    myExitsFrom = new QTreeWidget(this);
    myExitsFrom->setHeaderLabel(tr("Exits from this map"));
    layout->addWidget(myExitsFrom, 1, 0, 1, 2);

    myExitsTo = new QTreeWidget(this);
    myExitsTo->setHeaderLabel(tr("Exits leading to this map"));
    layout->addWidget(myExitsTo, 2, 0, 1, 2);
}

CREMapPanel::~CREMapPanel()
{
}

void CREMapPanel::setMap(CREMapInformation* map)
{
    myName->setText(map->name());

    myExitsFrom->clear();
    foreach(QString path, map->accessedFrom())
        myExitsFrom->addTopLevelItem(new QTreeWidgetItem(QStringList(path)));

    myExitsTo->clear();
    foreach(QString path, map->exitsTo())
        myExitsTo->addTopLevelItem(new QTreeWidgetItem(QStringList(path)));
}
