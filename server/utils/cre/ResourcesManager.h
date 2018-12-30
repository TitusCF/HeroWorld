#ifndef RESOURCESMANAGER_H
#define	RESOURCESMANAGER_H

extern "C" {
#include "global.h"
}

class ResourcesManager
{
    public:
        ResourcesManager();
        virtual ~ResourcesManager();

        void load();

        QStringList archetypes() const;
        const archt* archetype(const QString& name) const;

        QStringList treasureLists() const;
        const treasurelist* treasureList(const QString& name);

        int recipeMaxIngredients() const;
        QStringList recipes(int count) const;
        const recipestruct* recipe(int ingredients, const QString& name) const;

        QStringList faces() const;
        const New_Face* face(const QString& name) const;

        QStringList allAnimations() const;
        const animations_struct* animation(const QString& name) const;

    protected:
        QHash<QString, archt*> myArchetypes;
        QHash<QString, treasurelist*> myTreasures;
        QList<QHash<QString, recipestruct*> > myRecipes;
        QHash<QString, New_Face*> myFaces;
        QHash<QString, animations_struct*> myAnimations;
};

#endif	/* RESOURCESMANAGER_H */

