#include "CREWrapperFormulae.h"

CREWrapperFormulae::CREWrapperFormulae()
{
    myFormulae = NULL;
}

void CREWrapperFormulae::setFormulae(const recipe* rec)
{
    myFormulae = rec;
}

QString CREWrapperFormulae::fullname() const
{
    if (myFormulae->arch_names == 0)
    {
        return QString("%1 (no archetype?)").arg(myFormulae->title);
    }

    const archetype* base = find_archetype(myFormulae->arch_name[0]);
    if (strcmp(myFormulae->title, "NONE") == 0)
    {
        return base->clone.name;
    }

    return QString("%1 of %2").arg(base->clone.name, myFormulae->title);
}

QString CREWrapperFormulae::title() const
{
    return myFormulae->title;
}

int CREWrapperFormulae::chance() const
{
    return myFormulae->chance;
}

int CREWrapperFormulae::difficulty() const
{
    return myFormulae->diff;
}

int CREWrapperFormulae::experience() const
{
    return myFormulae->exp;
}

QStringList CREWrapperFormulae::archs() const
{
    QStringList archs;
    for (size_t i = 0; i < myFormulae->arch_names; i++)
    {
        archs.append(myFormulae->arch_name[i]);
    }
    return archs;
}

QStringList CREWrapperFormulae::ingredients() const
{
    QStringList ingredients;
    for (linked_char* ing = myFormulae->ingred; ing; ing = ing->next)
    {
        ingredients.append(ing->name);
    }
    return ingredients;
}
