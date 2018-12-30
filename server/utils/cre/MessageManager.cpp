#include "MessageManager.h"
#include "MessageFile.h"
#include "QuestConditionScript.h"

extern "C" {
    #include "global.h"
}

#include <QDir>
#include <QDebug>

MessageManager::MessageManager()
{
}

MessageManager::~MessageManager()
{
    qDeleteAll(myMessages);
    qDeleteAll(myPreConditions);
    qDeleteAll(myPostConditions);
}

void MessageManager::loadMessages()
{
    loadDirectory("");

    /* get pre and post conditions */
    findPrePost("pre", myPreConditions);
    findPrePost("post", myPostConditions);
}

void MessageManager::saveMessages()
{
    foreach(MessageFile* file, myMessages)
    {
        file->save();
    }
}

QList<MessageFile*>& MessageManager::messages()
{
    return myMessages;
}

const QList<MessageFile*>& MessageManager::messages() const
{
    return myMessages;
}

MessageFile* MessageManager::findMessage(const QString& path)
{
    foreach(MessageFile* file, myMessages)
    {
        if (file->path() == path)
            return file;
    }

    return NULL;
}

void MessageManager::loadDirectory(const QString& directory)
{
    //qDebug() << "load" << directory;
    QDir dir(QString("%1/%2/%3").arg(settings.datadir, settings.mapdir, directory));

    // first messages
    QStringList messages = dir.entryList(QStringList("*.msg"), QDir::Files);
    //qDebug() << "found" << messages;
    foreach(QString message, messages)
    {
        QString path = directory + QDir::separator() + message;
        MessageFile* file = new MessageFile(path);
        if (file->parseFile())
        {
            myMessages.append(file);
        }
        else
        {
            qDebug() << "dialog parse error" << path;
            delete file;
        }
    }

    // recurse
    QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach(QString sub, subdirs)
        loadDirectory(directory + QDir::separator() + sub);
}

QList<QuestConditionScript*> MessageManager::preConditions() const
{
    return myPreConditions;
}

QList<QuestConditionScript*> MessageManager::postConditions() const
{
    return myPostConditions;
}

QString MessageManager::loadScriptComment(const QString& path) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return "";

    QTextStream stream(&file);
    QStringList lines = stream.readAll().split("\n");

    QString comment, line;

    /* by convention, the first 2 lines are encoding and script name */
    for(int i = 2; i < lines.size(); i++)
    {
        line = lines[i];
        if (!line.startsWith("# "))
            break;
        comment += line.mid(2) + "\n";
    }

    return comment.trimmed();
}

void MessageManager::findPrePost(const QString directory, QList<QuestConditionScript*>& list)
{
    QDir dir(QString("%1/%2/python/dialog/%3").arg(settings.datadir, settings.mapdir, directory));
    QFileInfoList files = dir.entryInfoList(QStringList("*.py"));
    foreach(QFileInfo file, files)
    {
        list.append(new QuestConditionScript(file.baseName(), loadScriptComment(file.absoluteFilePath())));
    }
}
