#ifndef LOADGAMEDIALOG_H
#define LOADGAMEDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "../core/savesystem.h"

class LoadGameDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoadGameDialog(QWidget* parent = nullptr);
    ~LoadGameDialog();
    
    QString getSelectedSave() const;
    
private slots:
    void onLoadClicked();
    void onDeleteClicked();
    void onCancelClicked();
    void onSaveSelected(QListWidgetItem* item);
    
private:
    QListWidget* m_savesList;
    QPushButton* m_loadButton;
    QPushButton* m_deleteButton;
    QPushButton* m_cancelButton;
    QString m_selectedSave;
    
    void setupUI();
    void loadSaves();
};

#endif // LOADGAMEDIALOG_H
