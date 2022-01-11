#ifndef INSERTDIALOG_H
#define INSERTDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QButtonGroup>
#include <QString>
#include "utils.h"

namespace Ui {
class InsertDialog;
}

class InsertDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InsertDialog(QWidget *parent = nullptr);
    ~InsertDialog();
private slots:
    void on_finish_pushButton_clicked();
    void on_close_pushButton_clicked();
signals:
    void get_person_finished(util::person_info new_info);
    void get_person_canceled();
    void get_person_error();
private:
    Ui::InsertDialog *ui;
    QButtonGroup *btn_group;
};

#endif // INSERTDIALOG_H
