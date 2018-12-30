#ifndef _CRETREEITEMMESSAGE_H
#define	_CRETREEITEMMESSAGE_H

#include "CRETreeItem.h"
class MessageFile;

class CRETreeItemMessage : public CRETreeItem
{
    public:
        CRETreeItemMessage(MessageFile* message);
        virtual ~CRETreeItemMessage();

        virtual QString getPanelName() const  { return "Message"; }
        virtual void fillPanel(QWidget* panel);

    private:
        MessageFile* myMessage;
};

#endif	/* _CRETREEITEMMESSAGE_H */

