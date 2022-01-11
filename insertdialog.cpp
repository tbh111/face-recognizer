#include "insertdialog.h"
#include "ui_insertdialog.h"

InsertDialog::InsertDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InsertDialog)
{
    ui->setupUi(this);
    btn_group = new QButtonGroup;
    btn_group->setExclusive(true);
    btn_group->addButton(ui->L1_radioButton, 1);
    btn_group->addButton(ui->L2_radioButton, 2);
}

InsertDialog::~InsertDialog()
{
    delete ui;
}

void InsertDialog::on_finish_pushButton_clicked()
{
    QString name = ui->Stuff_lineEdit->text();
    QString id = ui->ID_lineEdit->text();
    int permission = btn_group->checkedId();
    if(name.isEmpty() && id.isEmpty())
    {
        util::person_info new_person;
        emit get_person_finished(new_person);
        QMessageBox::warning(this, "Warning","use default guest info");
        QDialog::accept();

    }
    else if(name.length() > 10 || id.length() != 6)
    {
        emit get_person_error();
        QMessageBox::warning(this, "Warning","please check your name and id");
        QDialog::reject();
    }
    else
    {
        util::person_info new_info(id, name, permission);
        emit get_person_finished(new_info);
        QString msg = QString("name: %1 id: %2 permission: %3 ").arg(name).arg(id).arg(permission);
        QMessageBox::information(this, "Info", msg);
        QDialog::accept();
    }
}

void InsertDialog::on_close_pushButton_clicked()
{
//    emit get_person_canceled();
    QDialog::reject();
}
