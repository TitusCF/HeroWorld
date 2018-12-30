#include "CREWrapperArtifact.h"

CREWrapperArtifact::CREWrapperArtifact()
{
    myArtifact = NULL;
}

void CREWrapperArtifact::setArtifact(const artifact* art)
{
    myArtifact = art;
    myItem.setObject(myArtifact->item);
}

QObject* CREWrapperArtifact::item()
{
    return &myItem;
}

int CREWrapperArtifact::chance() const
{
    return myArtifact->chance;
}

int CREWrapperArtifact::difficulty() const
{
    return myArtifact->difficulty;
}

QStringList CREWrapperArtifact::allowed() const
{
    QStringList allowed;
    linked_char* a = myArtifact->allowed;

    while (a)
    {
        allowed.append(a->name);
        a = a->next;
    }
    return allowed;
}
