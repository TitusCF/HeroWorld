#ifndef _CREPANEL_H
#define	_CREPANEL_H

#include <QObject>
#include <QWidget>

class CREPanel : public QWidget
{
    Q_OBJECT

    public:
        CREPanel();
        virtual ~CREPanel();

        virtual void commitData();
    private:
};

#endif	/* _CREPANEL_H */

