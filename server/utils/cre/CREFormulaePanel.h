#ifndef CREFORMULAEPANEL_H
#define CREFORMULAEPANEL_H

#include <QObject>
#include <QtGui>
#include "CREPanel.h"

extern "C" {
#include "global.h"
#include "recipe.h"
}

class CREFormulaePanel : public CREPanel
{
    Q_OBJECT

    public:
        CREFormulaePanel();

        void setRecipe(const recipe* recipe);

    protected:
        const recipe* myRecipe;
        QComboBox* myTitle;
        QLineEdit* myYield;
        QLineEdit* myChance;
        QLineEdit* myExperience;
        QLineEdit* myDifficulty;
        QComboBox* mySkill;
        QComboBox* myCauldron;
        QLabel* myIndex;
        QTreeWidget* myArchetypes;
        QTextEdit* myIngredients;
        QPushButton* myValidate;
        QPushButton* myReset;

    protected slots:
        void resetClicked(bool);
        void validateClicked(bool);
};

#endif // CREFORMULAEPANEL_H
