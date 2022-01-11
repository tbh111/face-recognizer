#include "removedialog.h"
#include "ui_removedialog.h"

RemoveDialog::RemoveDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RemoveDialog)
{
    ui->setupUi(this);
    btn_group = new QButtonGroup;
    btn_group->setExclusive(true);
    btn_group->addButton(ui->Finish_pushButton, 1);
    btn_group->addButton(ui->Cancel_pushButton, 2);
}

RemoveDialog::~RemoveDialog()
{
    delete ui;
}

void RemoveDialog::on_Finish_pushButton_clicked()
{
    QString id = ui->ID_lineEdit->text();
    if(id.isEmpty() && id.length() > 6)
    {
        emit get_person_error();
        QMessageBox::warning(this, "Warning","please check your name and id");
        QDialog::reject();
    }
    else
    {
        emit get_person_finished(id);
        QString msg = QString("id: %1").arg(id);
        QMessageBox::information(this, "Info", msg);
        QDialog::accept();
    }
}

void RemoveDialog::on_Cancel_pushButton_clicked()
{
    QDialog::reject();
}
