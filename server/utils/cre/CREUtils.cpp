#include <QtGui>
#include "CREUtils.h"
#include "CREPixmap.h"
#include "CREMapInformation.h"
#include "Quest.h"
#include "MessageFile.h"

extern "C" {
#include "global.h"
#include "recipe.h"
#include "libproto.h"
}

QTreeWidgetItem* CREUtils::archetypeNode(QTreeWidgetItem* parent)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList(QTreeWidget::tr("Archetypes")));
    return item;
}

QTreeWidgetItem* CREUtils::archetypeNode(const archt* arch, QTreeWidgetItem* parent)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList(arch->name));
    //item->setData(0, Qt::UserRole, arch->getId());
    item->setIcon(0, CREPixmap::getIcon(arch->clone.face->number));
    return item;
}

QTreeWidgetItem* CREUtils::artifactNode(QTreeWidgetItem* parent)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList(QTreeWidget::tr("Artifacts")));
    return item;
}

QTreeWidgetItem* CREUtils::artifactNode(const artifact* arti, QTreeWidgetItem* parent)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList(arti->item->name));
    if (arti->item->face != NULL && arti->item->face != blank_face)
        item->setIcon(0, CREPixmap::getIcon(arti->item->face->number));
    else
    {
        int face = 0;
        for (linked_char* allowed = arti->allowed; allowed; allowed = allowed->next)
        {
            if (allowed->name[0] == '!')
                continue;
            const archt* specific = try_find_archetype(allowed->name);
            if (!specific)
                specific = find_archetype_by_object_name(allowed->name);
            if (specific && specific->clone.face != NULL)
            {
                face = specific->clone.face->number;
                break;
            }
        }

        if (face == 0)
        {
            const archt* generic = get_archetype_by_type_subtype(arti->item->type, -1);
            if (generic != NULL && generic->clone.face != NULL)
                face = generic->clone.face->number;
        }

        if (face)
            item->setIcon(0, CREPixmap::getIcon(face));
    }
    return item;
}

QTreeWidgetItem* CREUtils::treasureNode(QTreeWidgetItem* parent)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList(QTreeWidget::tr("Treasures")));
    return item;
}

QTreeWidgetItem* CREUtils::treasureNode(const treasurelist* list, QTreeWidgetItem* parent)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList(list->name));
    if (list->total_chance == 0)
        item->setIcon(0, CREPixmap::getTreasureIcon());
    else
        item->setIcon(0, CREPixmap::getTreasureOneIcon());
    return item;
}

QTreeWidgetItem* CREUtils::treasureNode(const treasure* treasure, const treasurelist* list, QTreeWidgetItem* parent)
{
    QTreeWidgetItem* item;
    if (treasure->item)
    {
        item = CREUtils::archetypeNode(treasure->item, parent);
        if (treasure->next_yes)
        {
            QTreeWidgetItem* node = new QTreeWidgetItem(item, QStringList(QTreeWidget::tr("Yes")));
            node->setIcon(0, CREPixmap::getTreasureYesIcon());
            CREUtils::treasureNode(treasure->next_yes, list, node);
        }
        if (treasure->next_no)
        {
            QTreeWidgetItem* node = new QTreeWidgetItem(item, QStringList(QTreeWidget::tr("No")));
            node->setIcon(0, CREPixmap::getTreasureNoIcon());
            CREUtils::treasureNode(treasure->next_no, list, node);
        }
    }
    else
    {
        if (treasure->name && strcmp(treasure->name, "NONE") == 0)
            item = new QTreeWidgetItem(parent, QStringList(QString(QTreeWidget::tr("Nothing"))));
        else
        {
            treasurelist* other = find_treasurelist(treasure->name);
            item = CREUtils::treasureNode(other, parent);
        }
    }

    if (list->total_chance != 0)
    {
        item->setText(0, QTreeWidget::tr("%1 (%2%, %3 chances on %4)").arg(item->text(0)).arg(qRound(100 * treasure->chance / list->total_chance)).arg(treasure->chance).arg(list->total_chance));
    }

    return item;
}

QTreeWidgetItem* CREUtils::formulaeNode(const recipe* recipe, QTreeWidgetItem* parent)
{
    QString title;
    const archt* base = NULL;

    if (recipe->arch_names == 0)
    {
        title = QString("%1 (no archetype?)").arg(recipe->title);
    }
    else
    {
        base = find_archetype(recipe->arch_name[0]);
        if (strcmp(recipe->title, "NONE") == 0)
        {
            title = base->clone.name;
        }
        else
        {
            title = QString("%1 of %2").arg(base->clone.name, recipe->title);
        }
    }
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList(title));
    if (base != NULL && base->clone.face != 0)
        item->setIcon(0, CREPixmap::getIcon(base->clone.face->number));

    return item;
}

QTreeWidgetItem* CREUtils::faceNode(QTreeWidgetItem* parent)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList(QTreeWidget::tr("Faces")));
    return item;
}

QTreeWidgetItem* CREUtils::faceNode(const New_Face* face, QTreeWidgetItem* parent)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList(face->name));
    item->setIcon(0, CREPixmap::getIcon(face->number));
    return item;
}

QTreeWidgetItem* CREUtils::animationNode(QTreeWidgetItem* parent)
{
    return new QTreeWidgetItem(parent, QStringList(QTreeWidget::tr("Animations")));
}

QTreeWidgetItem* CREUtils::animationNode(const Animations* anim, QTreeWidgetItem* parent)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList(anim->name));
    if (anim->num_animations > 0)
        item->setIcon(0, CREPixmap::getIcon((anim->faces[0] != NULL) ? anim->faces[0]->number : 0));
    else
        LOG(llevDebug, "empty animation: %s\n", anim->name);
    return item;
}

QTreeWidgetItem* CREUtils::regionNode(const QString& name, int count, QTreeWidgetItem *parent)
{
    return new QTreeWidgetItem(parent, QStringList(QObject::tr("%1 - %2 maps").arg(name).arg(count)));
}

QTreeWidgetItem* CREUtils::mapNode(QTreeWidgetItem *parent)
{
  return new QTreeWidgetItem(parent, QStringList(QTreeWidget::tr("Maps")));
}

QTreeWidgetItem* CREUtils::mapNode(const CREMapInformation* map, QTreeWidgetItem *parent)
{
    return new QTreeWidgetItem(parent, QStringList(QObject::tr("%1 [%2]").arg(map->name(), map->path())));
}

QTreeWidgetItem* CREUtils::questsNode()
{
  return new QTreeWidgetItem(QStringList(QTreeWidget::tr("Quests")));
}

QTreeWidgetItem* CREUtils::questNode(const Quest* quest, QTreeWidgetItem* parent)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList(quest->code()));
    if (quest->faceNumber() != 0)
      item->setIcon(0, CREPixmap::getIcon(quest->faceNumber()));
    return item;
}

QTreeWidgetItem* CREUtils::messagesNode()
{
  return new QTreeWidgetItem(QStringList(QTreeWidget::tr("NPC dialogs")));
}

QTreeWidgetItem* CREUtils::messageNode(const MessageFile* message, QTreeWidgetItem* parent)
{
    return new QTreeWidgetItem(parent, QStringList(message->path()));
}
