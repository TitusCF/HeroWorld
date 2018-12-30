#ifndef CLASS_CRE_MAP_INFORMATION_H
#define CLASS_CRE_MAP_INFORMATION_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QHash>

class CREMapInformation : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString path READ path)
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(int level READ level)
    Q_PROPERTY(qint64 experience READ experience)

    public:
        CREMapInformation();
        CREMapInformation(const QString& path);

        CREMapInformation* clone() const;

        const QString& path() const;
        void setPath(const QString& path);

        const QString& name() const;
        void setName(const QString& name);

        QStringList archetypes() const;
        void addArchetype(const QString& archetype);

        const QDateTime& mapTime() const;
        void setMapTime(const QDateTime& date);

        QStringList exitsTo() const;
        void addExitTo(const QString& path);

        QStringList accessedFrom() const;
        void addAccessedFrom(const QString& path);

        int level() const;
        void setLevel(int level);

        qint64 experience() const;
        void setExperience(qint64 experience);

        const QString& region() const;
        void setRegion(const QString& region);

        QStringList messages() const;
        void addMessage(const QString& message);

        QStringList quests() const;
        void addQuest(const QString& quest);

        QHash<QString, int>& shopItems();
        const QHash<QString, int>& shopItems() const;

        double shopGreed() const;
        void setShopGreed(double greed);
        const QString& shopRace() const;
        void setShopRace(const QString& race);
        quint64 shopMin() const;
        void setShopMin(quint64 min);
        quint64 shopMax() const;
        void setShopMax(quint64 max);

    protected:
        QString myPath;
        QString myName;
        QStringList myArchetypes;
        QDateTime myMapTime;
        QStringList myExitsTo;
        QStringList myAccessedFrom;
        int myLevel;
        qint64 myExperience;
        QString myRegion;
        QStringList myMessages;
        QStringList myQuests;
        QHash<QString, int> myShopItems;
        double myShopGreed;
        QString myShopRace;
        quint64 myShopMin, myShopMax;

        void copy(const CREMapInformation& other);
};

#endif // CLASS_CRE_MAP_INFORMATION_H
