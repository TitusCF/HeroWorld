#ifndef CRE_WRAPPER_ARTIFACT_H
#define CRE_WRAPPER_ARTIFACT_H

#include <QObject>
#include <QStringList>

extern "C" {
#include "global.h"
}

#include "CREWrapperObject.h"

class CREWrapperArtifact : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* item READ item)
    Q_PROPERTY(int chance READ chance)
    Q_PROPERTY(int difficulty READ difficulty)
    Q_PROPERTY(QStringList allowed READ allowed)

    public:
        CREWrapperArtifact();

        void setArtifact(const artifact* art);

        QObject* item();
        int chance() const;
        int difficulty() const;
        QStringList allowed() const;

    protected:
        const artifact* myArtifact;
        CREWrapperObject myItem;
};

#endif // CRE_WRAPPER_ARTIFACT_H
