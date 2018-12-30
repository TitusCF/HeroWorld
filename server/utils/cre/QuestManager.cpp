#include "QuestManager.h"
#include "CREMainWindow.h"
#include "Quest.h"

extern "C" {
#include <global.h>
}

QuestManager::QuestManager()
{
}

QuestManager::~QuestManager()
{
    qDeleteAll(myQuests);
    qDeleteAll(myFiles);
}

void QuestManager::loadQuests()
{
    loadQuestFile("world.quests");
    foreach(Quest* quest, myQuests)
        quest->setModified(false);
    qDebug() << "found quests:" << myQuests.size();
}

void QuestManager::loadQuestFile(const QString& filename)
{
    int i, in = 0; /* 0: quest file, 1: one quest, 2: quest description, 3: quest step, 4: step description, 5: setwhen */
    Quest *quest = NULL;
    char includefile[MAX_BUF];
    QuestStep *step = NULL;
    char final[MAX_BUF], read[MAX_BUF];
    FILE *file;
    StringBuffer *buf;

    snprintf(final, sizeof(final), "%s/%s/%s", settings.datadir, settings.mapdir, qPrintable(filename));
    file = fopen(final, "r");
    if (!file) {
        LOG(llevError, "Can't open %s for reading quests", qPrintable(filename));
        return;
    }

    while (fgets(read, sizeof(read), file) != NULL) {
        if (in == 5) {
            if (strcmp(read, "end_setwhen\n") == 0) {
                in = 3;
                continue;
            }

            read[strlen(read) - 1] = '\0';
            step->setWhen().append(read);

            continue;
        }
        if (in == 4) {
            if (strcmp(read, "end_description\n") == 0) {
                char *message;

                in = 3;

                message = stringbuffer_finish(buf);
                buf = NULL;

                step->setDescription(message);
                free(message);

                continue;
            }

            stringbuffer_append_string(buf, read);
            continue;
        }

        if (in == 3) {
            if (strcmp(read, "end_step\n") == 0) {
                step = NULL;
                in = 1;
                continue;
            }
            if (strcmp(read, "finishes_quest\n") == 0) {
                step->setCompletion(true);
                continue;
            }
            if (strcmp(read, "description\n") == 0) {
                buf = stringbuffer_new();
                in = 4;
                continue;
            }
            if (strcmp(read, "setwhen\n") == 0) {
                in = 5;
                continue;
            }
            LOG(llevError, "quests: invalid line %s in definition of quest %s in file %s!\n",
                    read, qPrintable(quest->code()), qPrintable(filename));
            continue;
        }

        if (in == 2) {
            if (strcmp(read, "end_description\n") == 0) {
                char *message;

                in = 1;

                message = stringbuffer_finish(buf);
                buf = NULL;

                quest->setDescription(message);
                free(message);

                continue;
            }
            stringbuffer_append_string(buf, read);
            continue;
        }

        if (in == 1) {
            if (strcmp(read, "end_quest\n") == 0) {
                quest = NULL;
                in = 0;
                continue;
            }

            if (strcmp(read, "description\n") == 0) {
                in = 2;
                buf = stringbuffer_new();
                continue;
            }

            if (strncmp(read, "title ", 6) == 0) {
                read[strlen(read) - 1] = '\0';
                quest->setTitle(read + 6);
                continue;
            }

            if (sscanf(read, "step %d\n", &i)) {
                step = new QuestStep();
                step->setStep(i);
                quest->steps().append(step);
                in = 3;
                continue;
            }

            if (sscanf(read, "restart %d\n", &i)) {
                quest->setRestart(i != 0);
                continue;
            }

            if (strncmp(read, "face ", 5) == 0) {
                read[strlen(read) - 1] = '\0';
                quest->setFace(read + 5);
                continue;
            }
        }

        if (read[0] == '#')
            continue;

        if (strncmp(read, "quest ", 6) == 0) {
            quest = new Quest();
            addQuest(filename, quest);
            read[strlen(read) - 1] = '\0';
            quest->setCode(read + 6);
            if (getByCode(quest->code()) != NULL) {
                LOG(llevError, "Quest %s is listed in file %s, but this quest has already been defined\n", read + 6, qPrintable(filename));
            }
            myQuests.append(quest);
            in = 1;
            continue;
        }
        if (sscanf(read, "include %s\n", includefile)) {
            char inc_path[HUGE_BUF], p[HUGE_BUF];
            snprintf(p, sizeof(p), qPrintable(filename));
            path_combine_and_normalize(p, includefile, inc_path, sizeof(inc_path));
            loadQuestFile(inc_path);
            myIncludes[filename].append(includefile);
            continue;
        }
        if (strncmp(read, "parent ", 7) == 0) {
            read[strlen(read) - 1] = '\0';
            Quest* parent = findByCode(read + 7);
            if (parent == NULL)
                LOG(llevError, "Quest %s was defined before its parent %s", qPrintable(quest->code()), read + 7);
            else
                quest->setParent(parent);
            continue;
        }

        if (strcmp(read, "\n") == 0)
            continue;

        LOG(llevError, "quest: invalid file format for %s, I don't know what to do with the line %s\n", final, read);
    }
}

Quest* QuestManager::getByCode(const QString& code)
{
    foreach(Quest* quest, myQuests)
    {
        if (quest->code() == code)
            return quest;
    }
    return NULL;
}

QList<const Quest*> QuestManager::quests() const
{
    QList<const Quest*> quests;
    foreach(const Quest* quest, myQuests)
        quests.append(quest);
    return quests;
}

QList<Quest*>& QuestManager::quests()
{
    return myQuests;
}

void QuestManager::addQuest(const QString& filename, Quest* quest)
{
    if (!myFiles.contains(filename))
        myFiles[filename] = new QList<Quest*>();
    myFiles[filename]->append(quest);
}

void QuestManager::saveQuests()
{
    foreach(const QString& filename, myFiles.keys())
        saveQuestFile(filename);
}

void QuestManager::saveQuestFile(const QString& filename)
{
    QList<Quest*>* list = myFiles[filename];
    bool modified = false;
    foreach(Quest* quest, *list)
    {
        if (quest->isModified())
        {
            modified = true;
            break;
        }
    }

    if (!modified)
        return;

    QString path = QString("%1/%2/%3").arg(settings.datadir, settings.mapdir, filename);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug() << "save error quest file" << filename << path;
        /** @todo be smarter */
        return;
    }

    qDebug() << "saving quests" << filename;

    QTextStream stream(&file);

    QStringList includes = myIncludes[filename];
    if (includes.size() > 0)
    {
        foreach(QString include, includes)
            stream << "include " << include << "\n";
        stream << "\n\n";
    }

    foreach(Quest* quest, *list)
    {
        stream << "quest " << quest->code() << "\n";
        if (!quest->title().isEmpty())
            stream << "title " << quest->title() << "\n";
        if (!quest->face().isEmpty())
            stream << "face " << quest->face() << "\n";
        if (!quest->description().isEmpty())
        {
            stream << "description\n" << quest->description();
            if (!quest->description().endsWith("\n"))
                stream << "\n";
            stream << "end_description\n";
        }
        if (quest->parent() != NULL)
            stream << "parent " << quest->parent()->code() << "\n";
        if (quest->canRestart())
            stream << "restart 1\n";

        foreach(QuestStep* step, quest->steps())
        {
            stream << "step " << step->step() << "\n";
            if (step->isCompletion())
                stream << "finishes_quest\n";
            if (!step->description().isEmpty())
            {
                stream << "description\n" << step->description();
                if (!step->description().endsWith("\n"))
                    stream << "\n";
                stream << "end_description\n";
            }
            if (step->setWhen().size() > 0)
            {
                stream << "setwhen\n";
                foreach(QString when, step->setWhen())
                {
                    stream << when << "\n";
                }
                stream << "end_setwhen\n";
            }
            stream << "end_step\n";

        }

        stream << "end_quest\n\n";
    }
}

QStringList QuestManager::getFiles() const
{
    return myFiles.keys();
}

QString QuestManager::getQuestFile(Quest* quest) const
{
    foreach(QString file, myFiles.keys())
    {
        if (myFiles[file]->contains(quest))
            return file;
    }
    return QString();
}

void QuestManager::setQuestFile(Quest* quest, const QString& file)
{
    if (file.isEmpty())
        return;

    Q_ASSERT(getQuestFile(quest).isEmpty());
    addQuest(file, quest);
}

Quest* QuestManager::findByCode(const QString& code)
{
    foreach(Quest* quest, myQuests)
    {
        if (quest->code() == code)
            return quest;
    }
    return NULL;
}
