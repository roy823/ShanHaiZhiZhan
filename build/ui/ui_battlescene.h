/********************************************************************************
** Form generated from reading UI file 'battlescene.ui'
**
** Created by: Qt User Interface Compiler version 6.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BATTLESCENE_H
#define UI_BATTLESCENE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_BattleScene
{
public:

    void setupUi(QWidget *BattleScene)
    {
        if (BattleScene->objectName().isEmpty())
            BattleScene->setObjectName("BattleScene");
        BattleScene->resize(800, 600);

        retranslateUi(BattleScene);

        QMetaObject::connectSlotsByName(BattleScene);
    } // setupUi

    void retranslateUi(QWidget *BattleScene)
    {
        BattleScene->setWindowTitle(QCoreApplication::translate("BattleScene", "\346\210\230\346\226\227", nullptr));
    } // retranslateUi

};

namespace Ui {
    class BattleScene: public Ui_BattleScene {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BATTLESCENE_H
