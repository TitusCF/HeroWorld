#include <qboxlayout.h>
#include <qtreewidget.h>

#include "CREExperienceWindow.h"
#include "CREFilterDialog.h"

extern "C" {
#include "global.h"
}

CREExperienceWindow::CREExperienceWindow()
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QTreeWidget* tree = new QTreeWidget(this);
    tree->setColumnCount(3);
    QStringList headers;
    headers << tr("Level") << tr("Experience") << tr("Difference");
    tree->setHeaderLabels(headers);

    layout->addWidget(tree);

    extern sint64 *levels;
    sint64 previous = 0;

    for (int level = 0; level < settings.max_level + 1; level++)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(tree);
        item->setText(0, QString::number(level));
        item->setText(1, QString::number(levels[level]));
        item->setText(2, QString::number(levels[level] - previous));
        previous = levels[level];
    }

    setWindowTitle(tr("Experience"));
}

CREExperienceWindow::~CREExperienceWindow()
{
}
