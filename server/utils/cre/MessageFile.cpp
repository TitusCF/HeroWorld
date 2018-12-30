#include <qiodevice.h>
#include <qfile.h>
#include <qscriptengine.h>
#include <QDebug>

#include "MessageFile.h"

extern "C" {
    #include "global.h"
}

MessageRule::MessageRule()
{
    myIsModified = false;
}

MessageRule::MessageRule(const MessageRule& original) : QObject()
{
    myIsModified = true;
    myMatch = original.match();
    myPreconditions = original.preconditions();
    myMessages = original.messages();
    myPostconditions = original.postconditions();
    myReplies = original.replies();
}

MessageRule::~MessageRule()
{
}

const QString& MessageRule::comment() const
{
    return myComment;
}

void MessageRule::setComment(const QString& comment)
{
    myComment = comment;
}

const QStringList& MessageRule::match() const
{
    return myMatch;
}

QStringList& MessageRule::match()
{
    return myMatch;
}

void MessageRule::setMatch(const QStringList& match)
{
    myMatch = match;
}

const QList<QStringList>& MessageRule::preconditions() const
{
    return myPreconditions;
}

void MessageRule::setPreconditions(const QList<QStringList>& preconditions)
{
    myPreconditions = preconditions;
}

const QList<QStringList>& MessageRule::postconditions() const
{
    return myPostconditions;
}

void MessageRule::setPostconditions(const QList<QStringList>& postconditions)
{
    myPostconditions = postconditions;
}

const QStringList& MessageRule::messages() const
{
    return myMessages;
}

void MessageRule::setMessages(const QStringList& messages)
{
    myMessages = messages;
}

const QStringList& MessageRule::include() const
{
    return myInclude;
}

void MessageRule::setInclude(const QStringList& include)
{
    myInclude = include;
}

const QList<QStringList>& MessageRule::replies() const
{
    return myReplies;
}

void MessageRule::setReplies(const QList<QStringList>& replies)
{
    myReplies = replies;
}

bool MessageRule::isModified() const
{
    return myIsModified;
}

void MessageRule::setModified(bool modified)
{
    myIsModified = modified;
}

MessageFile::MessageFile(const QString& path)
{
    myPath = path;
    myIsModified = false;
}

MessageFile::~MessageFile()
{
    qDeleteAll(myRules);
}

const QString& MessageFile::location() const
{
    return myLocation;
}

const QString& MessageFile::path() const
{
    return myPath;
}

void MessageFile::setPath(const QString& path)
{
    if (myPath != path)
    {
        myPath = path;
        setModified();
    }
}

void MessageFile::setLocation(const QString& location)
{
    if (myLocation != location)
    {
        myLocation = location;
        setModified();
    }
}

void convert(QScriptValue& value, QList<QStringList>& list)
{
    list.clear();
    int length = value.property("length").toInt32();
    for (int l = 0; l < length; l++)
    {
        QStringList items;
        QScriptValue sub = value.property(l);

        int subl = sub.property("length").toInt32();
        for (int s = 0; s < subl; s++)
            items.append(sub.property(s).toString());

        list.append(items);
    }
}

bool MessageFile::parseFile()
{
    QString full = QString("%1/%2/%3").arg(settings.datadir, settings.mapdir, myPath);
    QFile file(full);
    file.open(QIODevice::ReadOnly);

    QByteArray data = file.readAll();
    if (data.size() == 0)
        return false;
    QString script = data;
    if (!script.startsWith('['))
        script = "[" + script + "]";

    QScriptValue value;
    QScriptEngine engine;
    value = engine.evaluate(script);
    if (engine.hasUncaughtException())
    {
        qDebug() << "message file evaluate error" << myPath << engine.uncaughtException().toString();
        return false;
    }

    QScriptValue first = value.property(0);
    myLocation = first.property("location").toString();

    QScriptValue rules = first.property("rules");
    int length = rules.property("length").toInt32();
    for (int r = 0; r < length; r++)
    {
        MessageRule* rule = new MessageRule();
        myRules.append(rule);
        QScriptValue v = rules.property(r);

        rule->setComment(v.property("comment").toString());

        QStringList items;
        qScriptValueToSequence(v.property("match"), items);
        rule->setMatch(items);

        QList<QStringList> lists;
        QScriptValue p = v.property("pre");
        convert(p, lists);
        rule->setPreconditions(lists);

        p = v.property("post");
        convert(p, lists);
        rule->setPostconditions(lists);

        items.clear();
        qScriptValueToSequence(v.property("include"), items);
        rule->setInclude(items);

        items.clear();
        qScriptValueToSequence(v.property("msg"), items);
        rule->setMessages(items);

        p = v.property("replies");
        convert(p, lists);
        rule->setReplies(lists);
    }
    return true;
}

QList<MessageRule*>& MessageFile::rules()
{
    return myRules;
}

QList<CREMapInformation*>& MessageFile::maps()
{
    return myMaps;
}

QString convert(const QString& text)
{
    QString tmp(text);
    return tmp.replace('"', "\\\"").replace('\n', "\\n");
}

QString convert(const QStringList& list)
{
    QStringList one;
    foreach(const QString& line, list)
    {
        one.append("\"" + convert(line) + "\"");
    }
    return "[" + one.join(", ") + "]";
}

QString convert(const QList<QStringList>& data)
{
    QStringList lines;

    foreach(const QStringList& list, data)
    {
        lines.append(convert(list));
    }

    return "[" + lines.join(", ") + "]";
}

QString convert(const MessageRule* rule)
{
    QString result;

    if (!rule->include().isEmpty())
    {
        result += "{\n  \"include\" : " + convert(rule->include());
        if (!rule->preconditions().isEmpty())
            result += ",\n  \"pre\" : " + convert(rule->preconditions());
        result +=  "\n  }";
        return result;
    }

    result += "{\n";

    if (!rule->comment().isEmpty())
    {
        result += "  \"comment\" : \"";
        result += convert(rule->comment());
        result += "\",\n";
    }

    result += "  \"match\" : ";
    result += convert(rule->match());

    result += ",\n  \"pre\" : ";
    result += convert(rule->preconditions());

    result += ",\n  \"post\" : ";
    result += convert(rule->postconditions());

    result += ",\n  \"msg\" : ";
    result += convert(rule->messages());

    if (!rule->replies().isEmpty())
    {
        result += ",\n  \"replies\" : ";
        result += convert(rule->replies());
    }

    result += "\n  }";

    return result;
}

void MessageFile::save()
{
    bool one = myIsModified;
    foreach(MessageRule* rule, myRules)
    {
        if (rule->isModified())
        {
            one = true;
            break;
        }
    }
    if (!one)
        return;

    qDebug() << "MessageFile saving" << myPath;

    QString data = "{\n";
    if (!myLocation.isEmpty())
        data += "  \"location\" : \"" + myLocation + "\",\n";
    data += "  \"rules\": [\n  ";

    QStringList rules;
    foreach(MessageRule* rule, myRules)
    {
        rules.append(convert(rule));
        rule->setModified(false);
    }

    data += rules.join(", ");
    data += "\n]}\n";

    QString full = QString("%1/%2/%3").arg(settings.datadir, settings.mapdir, myPath);
    QFile file(full);
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    file.write(data.toAscii());
    file.close();

    setModified(false);
}

bool MessageFile::isModified() const
{
    return myIsModified;
}

void MessageFile::setModified(bool modified)
{
    myIsModified = modified;
}
