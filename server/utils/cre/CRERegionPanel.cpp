#include <QtGui>

#include "CRERegionPanel.h"

CRERegionPanel::CRERegionPanel()
{
    QGridLayout* layout = new QGridLayout(this);

    int line = 0;
    layout->addWidget(new QLabel(tr("Short name:"), this), 0, 0);

    myShortName = new QLabel(this);
    layout->addWidget(myShortName, line++, 1);

    layout->addWidget(new QLabel(tr("Long name:"), this), line, 0);

    myName = new QLabel(this);
    layout->addWidget(myName, line++, 1);

    layout->addWidget(new QLabel(tr("Message:"), this), line, 0);
    myMessage = new QLabel(this);
    myMessage->setWordWrap(true);
    layout->addWidget(myMessage, line++, 1);

    layout->addWidget(new QLabel(tr("Jail:"), this), line, 0);

    myJail = new QLabel(this);
    myJailX = new QLabel(this);
    myJailY = new QLabel(this);

    layout->addWidget(myJail, line++, 1);
    layout->addWidget(myJailX, line++, 1);
    layout->addWidget(myJailY, line++, 1);
}

CRERegionPanel::~CRERegionPanel()
{
}

void CRERegionPanel::setRegion(regiondef* region)
{
    myShortName->setText(region->name);
    myName->setText(get_region_longname(region));
    myMessage->setText(get_region_msg(region));

    while (region && region->jailmap == NULL)
        region = region->parent;

    if (region)
    {
        myJail->setText(region->jailmap);
        myJailX->setText(QString::number(region->jailx));
        myJailY->setText(QString::number(region->jaily));
    }
    else
    {
        myJail->setText("?");
        myJailX->setText("?");
        myJailY->setText("?");
    }
}
