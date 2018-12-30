#include "CRESmoothFaceMaker.h"
#include "CREPixmap.h"
#include <QtGui>

extern "C" {
#include "global.h"
#include "image.h"
}

CRESmoothFaceMaker::CRESmoothFaceMaker()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    myAutoClose = false;
    myOverwrite = false;

    QGridLayout* layout = new QGridLayout(this);
    int line = 0;

    layout->addWidget(new QLabel(tr("Face to use:"), this), line, 0);
    myFace = new QComboBox(this);
    layout->addWidget(myFace, line++, 1, 1, 2);
    for (unsigned int face = 1; face < nrofpixmaps; face++)
        myFace->addItem(CREPixmap::getIcon(face), new_faces[face].name, face);

    layout->addWidget(new QLabel(tr("Path of the picture to create:"), this), line, 0);
    myDestination = new QLineEdit();
    layout->addWidget(myDestination, line, 1);
    connect(myDestination, SIGNAL(textEdited(const QString &)), this, SLOT(destinationEdited(const QString&)));

    QPushButton* browse = new QPushButton(tr("Browse"), this);
    layout->addWidget(browse, line++, 2);
    connect(browse, SIGNAL(clicked(bool)), this, SLOT(browse(bool)));

    QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Close, Qt::Horizontal, this);
    layout->addWidget(box, line++, 1, 1, 3);
    connect(box, SIGNAL(rejected()), this, SLOT(reject()));
    connect(box, SIGNAL(accepted()), this, SLOT(makeSmooth()));

    setWindowTitle(tr("Smooth face base generator"));

    QApplication::restoreOverrideCursor();
}

CRESmoothFaceMaker::~CRESmoothFaceMaker()
{
}

int CRESmoothFaceMaker::selectedFace() const
{
    return myFace->itemData(myFace->currentIndex()).toInt();
}

void CRESmoothFaceMaker::setSelectedFace(int face)
{
    int line = myFace->findData(face);
    if (line != -1)
        myFace->setCurrentIndex(line);
}

QString CRESmoothFaceMaker::destination() const
{
    return myDestination->text();
}

void CRESmoothFaceMaker::setAutoClose(bool autoClose)
{
    myAutoClose = autoClose;
}

void CRESmoothFaceMaker::makeSmooth()
{
    if (destination().isEmpty())
    {
        QMessageBox::warning(this, tr("Oops"), tr("You must select a destination!"));
        return;
    }

    if (!myOverwrite && QFile::exists(destination()))
    {
        if (QMessageBox::question(this, tr("Confirm file overwrite"), tr("File %1 already exists. Overwrite it?").arg(destination()), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
            return;
    }

    QPixmap pic(32 * 16, 32 * 2);
    pic.fill(QColor(0, 0, 0, 0));
    QImage icon = CREPixmap::getIcon(selectedFace()).pixmap(32, 32).toImage();
    QPainter device(&pic);
    device.setBackground(QColor(0, 0, 0, 0));

    long spots[] = {
        0x0,    /* 0b000000000 */
        0x49,   /* 0b001001001 */
        0x7,    /* 0b000000111 */
        0x4f,   /* 0b001001111 */
        0x124,  /* 0b100100100 */
        0x16d,  /* 0b101101101 */
        0x127,  /* 0b100100111 */
        0x16f,  /* 0b101101111 */
        0x1c0,  /* 0b111000000 */
        0x1c9,  /* 0b111001001 */
        0x1c7,  /* 0b111000111 */
        0x1cf,  /* 0b111001111 */
        0x1e4,  /* 0b111100100 */
        0x1ed,  /* 0b111101101 */
        0x1e7,  /* 0b111100111 */
        0x1ef,  /* 0b111101111 */
        0x0,    /* 0b000000000 */
        0x1,    /* 0b000000001 */
        0x4,    /* 0b000000100 */
        0x5,    /* 0b000000101 */
        0x100,  /* 0b100000000 */
        0x101,  /* 0b100000001 */
        0x104,  /* 0b100000100 */
        0x105,  /* 0b100000101 */
        0x40,   /* 0b001000000 */
        0x41,   /* 0b001000001 */
        0x44,   /* 0b001000100 */
        0x45,   /* 0b001000101 */
        0x140,  /* 0b101000000 */
        0x141,  /* 0b101000001 */
        0x144,  /* 0b101000100 */
        0x145,  /* 0b101000101 */
    };


    int sx, sy, sw, sh, dx, dy, spot;
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 2; y++) {
            spot = y * 16 + x;

            for (int bit = 0; bit < 9; bit++) {
                if ((spots[spot] & (1 << bit)) == 0)
                    continue;

                dx = x * 32;
                dy = y * 32;
                sw = 8;
                sh = 8;

                switch (bit / 3)
                {
                    case 0:
                        sy = 0;
                        break;
                    case 1:
                        sy = 8;
                        dy += 8;
                        sh = 16;
                        break;
                    case 2:
                        sy = 24;
                        dy += 24;
                        break;
                    default:
                        Q_ASSERT(false);
                }

                switch (bit % 3)
                {
                    case 0:
                        sx = 0;
                        break;
                    case 1:
                        sx = 8;
                        dx += 8;
                        sw = 16;
                        break;
                    case 2:
                        sx = 24;
                        dx += 24;
                        break;
                    default:
                        Q_ASSERT(false);
                }

                device.drawImage(dx, dy, icon, sx, sy, sw, sh);
            }

        }
    }

    if (!pic.save(destination(), "PNG"))
    {
        QMessageBox::critical(this, tr("Error"), tr("Error while saving the picture as %1!").arg(destination()));
        return;
    }

    QMessageBox::information(this, tr("Smooth base saved"), tr("The smooth base was correctly saved as %1").arg(destination()));

    if (myAutoClose)
        accept();

    myOverwrite = false;
}

void CRESmoothFaceMaker::browse(bool)
{
    QString dest = QFileDialog::getSaveFileName(this, tr("Select destination file"), "", tr("PNG file (*.png);;All files (*.*)"));
    if (dest.isEmpty())
        return;

    myDestination->setText(dest);
    myOverwrite = true;
}

void CRESmoothFaceMaker::destinationEdited(const QString&)
{
    myOverwrite = false;
}
