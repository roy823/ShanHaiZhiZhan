#ifndef SAVEGAMEDIALOG_H
#define SAVEGAMEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class SaveGameDialog : public QDialog {
    Q_OBJECT

public:
    explicit SaveGameDialog(QWidget* parent = nullptr);
    ~SaveGameDialog();
    
    QString getSaveName() const;
    
private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onSaveNameChanged(const QString& text);
    
private:
    QLineEdit* m_saveNameEdit;
    QPushButton* m_saveButton;
    QPushButton* m_cancelButton;
    
    void setupUI();
};

#endif // SAVEGAMEDIALOG_H
