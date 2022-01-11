#ifndef REMOVEDIALOG_H
#define REMOVEDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QButtonGroup>
#include <QString>
#include <QDebug>
#include "utils.h"

namespace Ui {
class RemoveDialog;
}

class RemoveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoveDialog(QWidget *parent = nullptr);
    ~RemoveDialog();
private slots:
    void on_Finish_pushButton_clicked();

    void on_Cancel_pushButton_clicked();

signals:
    void get_person_finished(QString remove_id);
    void get_person_canceled();
    void get_person_error();
private:
    Ui::RemoveDialog *ui;
    QButtonGroup *btn_group;
};

#endif // REMOVEDIALOG_H
