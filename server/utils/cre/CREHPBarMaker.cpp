#include "CREHPBarMaker.h"
#include "CREPixmap.h"
#include <QtGui>
#include <qt4/QtCore/qiodevice.h>
#include <qt4/QtCore/qfile.h>

extern "C" {
#include "global.h"
}

CREHPBarMaker::CREHPBarMaker()
{
    QGridLayout* layout = new QGridLayout(this);
    int line = 0;

    layout->addWidget(new QLabel(tr("Path where to create items:"), this), line, 0);
    myDestination = new QLineEdit();
    layout->addWidget(myDestination, line, 1);

    QPushButton* browse = new QPushButton(tr("Browse"), this);
    layout->addWidget(browse, line++, 2);
    connect(browse, SIGNAL(clicked(bool)), this, SLOT(browse(bool)));

    layout->addWidget(new QLabel(tr("Archetype name:"), this), line, 0);
    myName = new QLineEdit();
    layout->addWidget(myName, line++, 1, 1, 2);

    layout->addWidget(new QLabel(tr("Color:"), this), line, 0);
    myColorSelect = new QPushButton();
    layout->addWidget(myColorSelect, line++, 1, 1, 2);
    connect(myColorSelect, SIGNAL(clicked(bool)), this, SLOT(selectColor(bool)));

    layout->addWidget(new QLabel(tr("Y position from top:"), this), line, 0);
    myShift = new QSpinBox(this);
    myShift->setRange(1, 31);
    layout->addWidget(myShift, line++, 1, 1, 2);

    layout->addWidget(new QLabel(tr("Bar height:"), this), line, 0);
    myHeight = new QSpinBox(this);
    myHeight->setRange(1, 31);
    myHeight->setValue(5);
    layout->addWidget(myHeight, line++, 1, 1, 2);

    QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Close, Qt::Horizontal, this);
    layout->addWidget(box, line++, 1, 1, 3);
    connect(box, SIGNAL(rejected()), this, SLOT(reject()));
    connect(box, SIGNAL(accepted()), this, SLOT(makeBar()));

    myColor = QColor::fromRgb(255, 0, 0, 180);
    adjustColor();

    setWindowTitle(tr("HP bar face generator"));
}

CREHPBarMaker::~CREHPBarMaker()
{
}

void CREHPBarMaker::makeBar()
{
    if (myDestination->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Oops"), tr("You must select a destination!"));
        return;
    }

    if (myName->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Oops"), tr("You must enter a name!"));
        return;
    }

    QString base = myDestination->text() + QDir::separator() + myName->text();

    if (QFile::exists(base + ".arc"))
    {
        if (QMessageBox::question(this, tr("Confirm file overwrite"), tr("File %1 already exists. Overwrite it?").arg(base + ".arc"), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
            return;
    }

    QFile arc(base + ".arc");
    if (!arc.open(QFile::Truncate | QFile::WriteOnly))
    {
        QMessageBox::critical(this, tr("Error"), tr("Error while opening the archetype file %1!").arg(arc.fileName()));
        return;
    }

    int shift = myShift->value();
    int height = myHeight->value();

    for (int value = 1; value <= 30; value++)
    {
        QString line = tr("Object %1_%2\nface %1_%2.111\nend\n").arg(myName->text()).arg(value);
        arc.write(line.toLocal8Bit());

        QPixmap pic(32, 32);
        pic.fill(QColor(0, 0, 0, 0));
        QPainter device(&pic);
        device.fillRect(1, shift, value, height, myColor);

        QString picName = base + "_" + QString::number(value) + ".base.111.png";

        if (!pic.save(picName, "PNG"))
        {
            QMessageBox::critical(this, tr("Error"), tr("Error while saving the picture %1!").arg(picName));
            return;
        }

    }

    arc.close();

    QMessageBox::information(this, tr("Bar created"), tr("The bar was correctly saved as %1").arg(arc.fileName()));
}

void CREHPBarMaker::browse(bool)
{
    QString dest = QFileDialog::getExistingDirectory(this, tr("Select destination directory"), "");
    if (dest.isEmpty())
        return;

    myDestination->setText(dest);
}

void CREHPBarMaker::adjustColor()
{
    const QString style("QPushButton { background-color : %1; }");
    myColorSelect->setStyleSheet(style.arg(myColor.name()));
}

void CREHPBarMaker::selectColor(bool)
{
    QColor color = QColorDialog::getColor(myColor, this, tr("Select bar color"), QColorDialog::ShowAlphaChannel);
    if (!color.isValid())
        return;
    myColor = color;
    adjustColor();
}
