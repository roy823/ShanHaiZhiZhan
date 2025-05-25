/********************************************************************************
** Form generated from reading UI file 'preparescene.ui'
**
** Created by: Qt User Interface Compiler version 6.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PREPARESCENE_H
#define UI_PREPARESCENE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PrepareScene
{
public:

    void setupUi(QWidget *PrepareScene)
    {
        if (PrepareScene->objectName().isEmpty())
            PrepareScene->setObjectName("PrepareScene");
        PrepareScene->resize(800, 600);

        retranslateUi(PrepareScene);

        QMetaObject::connectSlotsByName(PrepareScene);
    } // setupUi

    void retranslateUi(QWidget *PrepareScene)
    {
        PrepareScene->setWindowTitle(QCoreApplication::translate("PrepareScene", "\345\207\206\345\244\207", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PrepareScene: public Ui_PrepareScene {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PREPARESCENE_H
