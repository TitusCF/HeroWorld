#include "CREMapInformationManager.h"
#include "CRESettings.h"
#include "CREArchetypePanel.h"
#include "MessageManager.h"
#include "MessageFile.h"
#include "QuestManager.h"
#include "Quest.h"

extern "C" {
#include "global.h"
}

CREMapInformationManager::CREMapInformationManager(QObject* parent, MessageManager* messageManager, QuestManager* questManager) : QObject(parent)
{
    Q_ASSERT(messageManager != NULL);
    Q_ASSERT(questManager != NULL);
    myMessageManager = messageManager;
    myQuestManager = questManager;
}

CREMapInformationManager::~CREMapInformationManager()
{
    qDeleteAll(myInformation.values());
}

bool CREMapInformationManager::browseFinished() const
{
    return myWorker.isFinished();
}

void CREMapInformationManager::start()
{
    if (myWorker.isRunning())
        return;

    myWorker = QtConcurrent::run(this, &CREMapInformationManager::browseMaps);
}

void CREMapInformationManager::checkInventory(const object* item, CREMapInformation* information)
{
    FOR_INV_PREPARE(item, inv)
    {
        archetype *arch = find_archetype(inv->arch->name);
        addArchetypeUse(arch->name, information);
        information->addArchetype(arch->name);
        checkEvent(inv, information);
        checkInventory(inv, information);
    } FOR_INV_FINISH();
}

void CREMapInformationManager::process(const QString& path2)
{
    /*
     don't ask why, but the variable gets apparently destroyed on the myToProcess.append() when it reallocated values...
    so keep a copy to avoid messes
     */
    QString path(path2);

    if (myCancelled)
        return;

    emit browsingMap(path);
//    qDebug() << "processing" << path;
    CREMapInformation* information = getOrCreateMapInformation(path);

    char tmppath[MAX_BUF];
    create_pathname(path.toAscii(), tmppath, MAX_BUF);
    QFileInfo info(tmppath);

    if (!info.exists())
    {
//        qDebug() << "non existant map" << tmppath;
        return;
    }

    if (!information->mapTime().isNull() && information->mapTime() >= info.lastModified())
    {
        foreach(QString exit, information->exitsTo())
        {
            if (!myToProcess.contains(exit))
                myToProcess.append(exit);
        }
//        qDebug() << "skipping " << tmppath;
        return;
    }

    mapstruct *m = ready_map_name(path.toAscii(), 0);
//    qDebug() << "processing" << path << information->mapTime() << info.lastModified();
    information->setName(m->name);
    information->setMapTime(info.lastModified());
    if (m->region != NULL)
        information->setRegion(m->region->name);
    else
        information->setRegion("wilderness"); /** @todo get from config */
    information->setLevel(m->difficulty);

    information->setShopGreed(m->shopgreed);
    if (m->shopitems != NULL)
    {
        for (int i = 0; i < m->shopitems[0].index; i++)
        {
            information->shopItems().insert(QString(m->shopitems[i].name == NULL ? "*" : m->shopitems[i].name), m->shopitems[i].strength);
        }
    }
    if (m->shoprace != NULL)
      information->setShopRace(m->shoprace);
    information->setShopMin(m->shopmin);
    information->setShopMax(m->shopmax);

    char exit_path[500];
    quint64 exp = 0;
    struct stat stats;

    for (int x = 0; x < 4; x++)
        if (m->tile_path[x] != NULL) {
            path_combine_and_normalize(m->path, m->tile_path[x], exit_path, sizeof(exit_path));
            create_pathname(exit_path, tmppath, MAX_BUF);
            if (stat(tmppath, &stats)) {
                printf("  map %s doesn't exist in map %s, for tile %d.\n", exit_path, m->path, x);
            }

            QString exit = exit_path;
            if (!myToProcess.contains(exit))
                myToProcess.append(exit);

            CREMapInformation* other = getOrCreateMapInformation(path);
            Q_ASSERT(other);
            other->addAccessedFrom(path);
            information->addExitTo(exit_path);
        }

    for (int x = MAP_WIDTH(m)-1; x >= 0; x--)
    {
        for (int y = MAP_HEIGHT(m)-1; y >= 0; y--)
        {
            FOR_MAP_PREPARE(m, x, y, item)
            {
                {
                    archetype *arch = find_archetype(item->arch->name);
                    addArchetypeUse(arch->name, information);
                    information->addArchetype(arch->name);
                }

                checkInventory(item, information);

                if (item->type == EXIT || item->type == TELEPORTER || item->type == PLAYER_CHANGER) {
                    char ep[500];
                    const char *start;

                    if (!item->slaying) {
                        ep[0] = '\0';
                        /*if (warn_no_path)
                            printf(" exit without any path at %d, %d on %s\n", item->x, item->y, info->path);*/
                    } else {
                        memset(ep, 0, 500);
                        if (strcmp(item->slaying, "/!"))
                            strcpy(ep, EXIT_PATH(item));
                        else {
                            if (!item->msg) {
                                //printf("  random map without message in %s at %d, %d\n", info->path, item->x, item->y);
                            } else {
                                /* Some maps have a 'exit_on_final_map' flag, ignore it. */
                                start = strstr(item->msg, "\nfinal_map ");
                                if (!start && strncmp(item->msg, "final_map", strlen("final_map")) == 0)
                                    /* Message start is final_map, nice */
                                    start = item->msg;
                                if (start) {
                                    const char *end = strchr(start+1, '\n');

                                    start += strlen("final_map")+2;
                                    strncpy(ep, start, end-start);
                                }
                            }
                        }

                        if (strlen(ep)) {
                            path_combine_and_normalize(m->path, ep, exit_path, 500);
                            create_pathname(exit_path, tmppath, MAX_BUF);
                            struct stat stats;
                            if (stat(tmppath, &stats)) {
                                printf("  map %s doesn't exist in map %s, at %d, %d.\n", ep, m->path, item->x, item->y);
                            } else {
                                QString exit = exit_path;
                                if (!myToProcess.contains(exit))
                                    myToProcess.append(exit);

                                CREMapInformation* other = getOrCreateMapInformation(path);
                                Q_ASSERT(other);
                                other->addAccessedFrom(path);
                                information->addExitTo(exit_path);

#if 0
                                link = get_map_info(exit_path);
                                add_map(link, &info->exits_from);
                                add_map(info, &link->exits_to);

                                if (do_regions_link) {
                                    mapstruct *link = ready_map_name(exit_path, 0);

                                    if (link && link != m) {
                                        /* no need to link a map with itself. Also, if the exit points to the same map, we don't
                                        * want to reset it. */
                                        add_region_link(m, link, item->arch->clone.name);
                                        link->reset_time = 1;
                                        link->in_memory = MAP_IN_MEMORY;
                                        delete_map(link);
                                    }
                                }
#endif
                            }
                        }
                    }
                }

                if (QUERY_FLAG(item, FLAG_MONSTER))
                    exp += item->stats.exp;
            } FOR_MAP_FINISH();
        }
    }

    information->setExperience(exp);

    QMutexLocker lock(&myLock);
    if (m->region == NULL)
        qDebug() << "map without region" << m->name << m->path;
    myExperience[m->region ? m->region->name : "(undefined)"] += exp;

    m->reset_time = 1;
    m->in_memory = MAP_IN_MEMORY;
    delete_map(m);
}

void CREMapInformationManager::browseMaps()
{
    /** @todo clear memory */
    myInformation.clear();
    myArchetypeUse.clear();

    loadCache();

    myCancelled = false;
    myCurrentMap = 0;
    myToProcess.clear();
    myToProcess.append(QString(first_map_path));

    /* try to find race-specific start maps */
    if (first_map_ext_path[0] != 0)
    {
        char path[MAX_BUF], name[MAX_BUF];
        const archetype* arch = first_archetype;
        struct stat stats;
        while (arch)
        {
            if (arch->clone.type == PLAYER)
            {
                snprintf(name, sizeof(name), "%s/%s", first_map_ext_path, arch->name);
                create_pathname(name, path, sizeof(path));
                if (stat(path, &stats) == 0) {
                    myToProcess.append(name);
                }
            }
            arch = arch->next;
        }
    }

    while (myCurrentMap < myToProcess.size())
    {
        process(myToProcess[myCurrentMap]);
        myCurrentMap++;
        if (myCancelled)
            break;
    }

    storeCache();

    emit finished();

    /** @todo make nicer report */
    qDebug() << "experience repartition:";
    foreach(QString region, myExperience.keys())
    {
        qDebug() << region << myExperience[region];
    }

    qDebug() << myToProcess.size() << "maps processed";
}

void CREMapInformationManager::cancel()
{
    myCancelled = true;
    myWorker.waitForFinished();
}

QList<CREMapInformation*> CREMapInformationManager::allMaps()
{
    QMutexLocker lock(&myLock);
    return myInformation.values();
}

QList<CREMapInformation*> CREMapInformationManager::getArchetypeUse(const archetype* arch)
{
    QMutexLocker lock(&myLock);
    return myArchetypeUse.values(arch->name);
}

void CREMapInformationManager::loadCache()
{
    Q_ASSERT(myInformation.isEmpty());

    CRESettings settings;
    QFile file(settings.mapCacheDirectory() + QDir::separator() + "maps_cache.xml");
    file.open(QFile::ReadOnly);

    QXmlStreamReader reader(&file);
    bool hasMaps = false;
    CREMapInformation* map = NULL;

    while (!reader.atEnd())
    {
        reader.readNext();

        if (reader.isStartElement() && reader.name() == "maps")
        {
            int version = reader.attributes().value("version").toString().toInt();
            if (version < 1)
                return;
            hasMaps = true;
            continue;
        }

        if (!hasMaps)
            continue;

        if (reader.isStartElement() && reader.name() == "map")
        {
            map = new CREMapInformation();
            continue;
        }
        if (reader.isStartElement() && reader.name() == "path")
        {
            QString path = reader.readElementText();
            map->setPath(path);
            Q_ASSERT(!myInformation.contains(path));
            myInformation[path] = map;
            continue;
        }
        if (reader.isStartElement() && reader.name() == "name")
        {
            map->setName(reader.readElementText());
            continue;
        }
        if (reader.isStartElement() && reader.name() == "lastModified")
        {
            QString date = reader.readElementText();
            map->setMapTime(QDateTime::fromString(date, Qt::ISODate));
            continue;
        }
        if (reader.isStartElement() && reader.name() == "level")
        {
            map->setLevel(reader.readElementText().toInt());
        }
        if (reader.isStartElement() && reader.name() == "experience")
        {
            map->setExperience(reader.readElementText().toLongLong());
        }
        if (reader.isStartElement() && reader.name() == "region")
        {
            map->setRegion(reader.readElementText());
        }
        if (reader.isStartElement() && reader.name() == "arch")
        {
            QString arch = reader.readElementText();
            map->addArchetype(arch);
            addArchetypeUse(arch, map);
            continue;
        }
        if (reader.isStartElement() && reader.name() == "exitTo")
        {
            QString path = reader.readElementText();
            map->addExitTo(path);
            continue;
        }
        if (reader.isStartElement() && reader.name() == "accessedFrom")
        {
            QString path = reader.readElementText();
            map->addAccessedFrom(path);
            continue;
        }
        if (reader.isStartElement() && reader.name() == "messageFile")
        {
            QString file = reader.readElementText();
            map->addMessage(file);
            MessageFile* message = myMessageManager->findMessage(file);
            if (message != NULL)
                message->maps().append(map);
            continue;
        }
        if (reader.isStartElement() && reader.name() == "quest")
        {
            QString code = reader.readElementText();
            map->addQuest(code);
            Quest* quest = myQuestManager->findByCode(code);
            if (quest != NULL)
                quest->maps().append(map);
            continue;
        }
        if (reader.isStartElement() && reader.name() == "shopItem")
        {
            QString item = reader.attributes().value("name").toString();
            int strength = reader.readElementText().toInt();
            map->shopItems()[item] = strength;
        }
        if (reader.isStartElement() && reader.name() == "shopGreed")
        {
            double greed = reader.readElementText().toDouble();
            map->setShopGreed(greed);
        }
        if (reader.isStartElement() && reader.name() == "shopRace")
        {
            map->setShopRace(reader.readElementText());
        }
        if (reader.isStartElement() && reader.name() == "shopMin")
        {
            quint64 min = reader.readElementText().toULongLong();
            map->setShopMin(min);
        }
        if (reader.isStartElement() && reader.name() == "shopMax")
        {
            quint64 max = reader.readElementText().toULongLong();
            map->setShopMax(max);
        }
        if (reader.isEndElement() && reader.name() == "map")
        {
            map = NULL;
            continue;
        }
    }

//    qDebug() << "loaded maps from cache:" << myInformation.size();
}

void CREMapInformationManager::storeCache()
{
    CRESettings settings;
    QFile file(settings.mapCacheDirectory() + QDir::separator() + "maps_cache.xml");
    file.open(QFile::WriteOnly | QFile::Truncate);

    QXmlStreamWriter writer(&file);

    writer.setAutoFormatting(true);
    writer.writeStartDocument();

    writer.writeStartElement("maps");
    writer.writeAttribute("version", "1");

    QList<CREMapInformation*> maps = myInformation.values();
    foreach(CREMapInformation* map, maps)
    {
        writer.writeStartElement("map");
        writer.writeTextElement("path", map->path());
        writer.writeTextElement("name", map->name());
        writer.writeTextElement("lastModified", map->mapTime().toString(Qt::ISODate));
        writer.writeTextElement("level", QString::number(map->level()));
        writer.writeTextElement("experience", QString::number(map->experience()));
        writer.writeTextElement("region", map->region());
        foreach(QString arch, map->archetypes())
        {
            writer.writeTextElement("arch", arch);
        }
        foreach(QString path, map->exitsTo())
        {
            writer.writeTextElement("exitTo", path);
        }
        foreach(QString path, map->accessedFrom())
        {
            writer.writeTextElement("accessedFrom", path);
        }
        foreach(QString file, map->messages())
        {
            writer.writeTextElement("messageFile", file);
        }
        foreach(QString code, map->quests())
        {
            writer.writeTextElement("quest", code);
        }
        foreach(QString item, map->shopItems().keys())
        {
            writer.writeStartElement("shopItem");
            writer.writeAttribute("name", item);
            writer.writeCharacters(QString::number(map->shopItems()[item]));
            writer.writeEndElement();
        }
        if (map->shopGreed() != 0)
        {
            writer.writeTextElement("shopGreed", QString::number(map->shopGreed()));
        }
        if (!map->shopRace().isEmpty())
        {
          writer.writeTextElement("shopRace", map->shopRace());
        }
        if (map->shopMin() != 0)
        {
          writer.writeTextElement("shopMin", QString::number(map->shopMin()));
        }
        if (map->shopMax() != 0)
        {
          writer.writeTextElement("shopMax", QString::number(map->shopMax()));
        }
        writer.writeEndElement();
    }

    writer.writeEndElement();

    writer.writeEndDocument();
}

CREMapInformation* CREMapInformationManager::getOrCreateMapInformation(const QString& path)
{
    if (!myInformation.contains(path))
    {
        CREMapInformation* information = new CREMapInformation(path);
        myInformation[path] = information;
    }
    return myInformation[path];
}

void CREMapInformationManager::addArchetypeUse(const QString& name, CREMapInformation* map)
{
    QMutexLocker lock(&myLock);
    if (!myArchetypeUse.values(name).contains(map))
        myArchetypeUse.insert(name, map);
}

void CREMapInformationManager::checkEvent(const object* item, CREMapInformation* map)
{
    const QString slaying = "/python/dialog/npc_dialog.py";
    const QString python = "Python";

    if (item->type != EVENT_CONNECTOR || python != item->title)
        return;

    if (item->subtype == EVENT_SAY && slaying == item->slaying)
    {
        //qDebug() << "message event in" << map->path() << item->name;
        QString path = item->name;
        if (!path.startsWith('/'))
            path = '/' + path;

        MessageFile* message = myMessageManager->findMessage(path);
        if (message != NULL)
        {
            if (!message->maps().contains(map))
                message->maps().append(map);
            map->addMessage(path);
        } else
            qDebug() << "missing message file" << path << "in" << map->path();
    }

    if (QString(item->slaying).startsWith("/python/quests/"))
    {
        //qDebug() << "quest-related Python stuff";
        QStringList split = QString(item->name).split(' ', QString::SkipEmptyParts);
        if (split.length() > 1)
        {
            Quest* quest = myQuestManager->findByCode(split[0]);
            if (quest != NULL)
            {
                //qDebug() << "definitely quest" << split[0];
                map->addQuest(split[0]);
                if (!quest->maps().contains(map))
                    quest->maps().append(map);
            } else
                qDebug() << "missing quest" << split[0] << "in" << map->path();
        }
    }
}

QList<CREMapInformation*> CREMapInformationManager::getMapsForRegion(const QString& region)
{
    QList<CREMapInformation*> list;

    foreach(CREMapInformation* map, myInformation.values())
    {
        if (map->region() == region)
            list.append(map);
    }

    return list;
}
