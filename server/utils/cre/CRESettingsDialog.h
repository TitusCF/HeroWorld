#ifndef CLASS_CRE_SETTINGS_DIALOG_H
#define CLASS_CRE_SETTINGS_DIALOG_H

#include <QDialog>

class QLineEdit;
class CRESettings;

class CRESettingsDialog : public QDialog
{
    public:
        CRESettingsDialog(CRESettings* settings);

        QString mapCache() const;

    protected:
        QLineEdit* myMapCache;
};

#endif // CLASS_CRE_SETTINGS_DIALOG_H
