#include "CRESettingsDialog.h"
#include "CRESettings.h"
#include <QtGui>

CRESettingsDialog::CRESettingsDialog(CRESettings* settings)
{
    setWindowTitle(tr("CRE settings"));

    QGridLayout* layout = new QGridLayout();

    int line = 0;

    layout->addWidget(new QLabel(tr("Map cache directory:"), this), line, 0);
    myMapCache = new QLineEdit(this);
    myMapCache->setText(settings->mapCacheDirectory());
    if (myMapCache->text().isEmpty())
        myMapCache->setText(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
    layout->addWidget(myMapCache, line, 1);

    line++;

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(buttons, line, 0, 1, 2);

    setLayout(layout);
}

QString CRESettingsDialog::mapCache() const
{
    return myMapCache->text();
}
