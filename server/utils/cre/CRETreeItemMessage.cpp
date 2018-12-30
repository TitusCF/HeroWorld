#include "CRETreeItemMessage.h"
#include "CREMessagePanel.h"

CRETreeItemMessage::CRETreeItemMessage(MessageFile* message)
{
    myMessage = message;
}

CRETreeItemMessage::~CRETreeItemMessage()
{
}

void CRETreeItemMessage::fillPanel(QWidget* panel)
{
    Q_ASSERT(myMessage);
    CREMessagePanel* p = static_cast<CREMessagePanel*>(panel);
    p->setMessage(myMessage);
}
