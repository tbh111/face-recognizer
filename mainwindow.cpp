#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    cam = new Camera(0);
    recognizer_client = new Recognizer(Recognizer::DETECT, Recognizer::LBP_CASCADE);
    db = new Database();
    QButtonGroup *btn_group = new QButtonGroup;
    btn_group->setExclusive(true);
    btn_group->addButton(ui->collect_btn);
    btn_group->addButton(ui->detect_btn);
    btn_group->addButton(ui->recognize_btn);
    btn_group->addButton(ui->remove_btn);

    connect(cam, SIGNAL(img_captured(Mat)), recognizer_client, SLOT(get_image(Mat)), Qt::BlockingQueuedConnection);
    connect(recognizer_client, SIGNAL(process_image_finished(QImage)), this, SLOT(draw(QImage)), Qt::DirectConnection);
    connect(recognizer_client, SIGNAL(end_training()), this, SLOT(train_one_end()), Qt::QueuedConnection);
    connect(recognizer_client, SIGNAL(end_removing()), this, SLOT(remove_one_end()), Qt::QueuedConnection);
    connect(recognizer_client, SIGNAL(face_pass(int)), db, SLOT(lookup_info(int)), Qt::QueuedConnection);
    connect(db, SIGNAL(lookup_suceess(util::person_info)), this, SLOT(print_info(util::person_info)), Qt::DirectConnection);
    db->connect_database();
    cam->start();
    recognizer_client->start();
}

MainWindow::~MainWindow()
{
    qDebug() << "main thread stop";
    if(mutex.tryLock())
    {
        mutex.unlock();
    }
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // TODO: find a better method to exit multi-thread process
//    cam->requestInterruption();
//    cam->wait();
    qDebug() << "close thread";
    mutex.lock();
    recognizer_client->recognizer_enable = false;
    cam->camera_enable = false;
    mutex.unlock();
//    recognizer_client->wait();
    cam->terminate();
    recognizer_client->terminate();
    qDebug() << "wait end";
    event->accept();
    delete cam;
    delete recognizer_client;
}

void MainWindow::print_info(util::person_info info)
{
    ui->text_id->setText(info.id_info);
    ui->text_name->setText(info.name_info);
    ui->text_permission->setText(QString::number(info.permission_info));
    ui->text_time->setText(info.last_login_info);
}

void MainWindow::train_one_end()
{
    // After training, ask if more stuff need to be added
    // if yes, reopen the dialog and input messages
    QMessageBox::StandardButton result = QMessageBox::question(this, "add new stuff", "anymore stuff to add?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if(result == QMessageBox::Yes)
    {
        qDebug() << "add more stuff";
        ins_dialog = new InsertDialog(this);
        qRegisterMetaType<util::person_info>("util::person_info");
        connect(ins_dialog, SIGNAL(get_person_finished(util::person_info)), this, SLOT(insert_new(util::person_info)), Qt::QueuedConnection);
        connect(db, SIGNAL(update_success(util::person_info)), recognizer_client, SLOT(update_info(util::person_info)), Qt::QueuedConnection);
        int state = ins_dialog->exec();
        if(state == QDialog::Accepted)
        {
            recognizer_client->switch_mode(Recognizer::GET_FACE_DATA);
        }
        else
        {
            recognizer_client->switch_mode(Recognizer::DETECT);
        }
    }
    else if(result == QMessageBox::No)
    {
        qDebug() << "exit add mode";
        ui->detect_btn->setChecked(true);
        recognizer_client->switch_mode(Recognizer::DETECT);
    }
    else
    {
        qDebug() << "exit add mode";
        ui->detect_btn->setChecked(true);
        recognizer_client->switch_mode(Recognizer::DETECT);
    }
}

void MainWindow::remove_one_end()
{
    QMessageBox::StandardButton result = QMessageBox::question(this, "remove stuff", "anymore stuff to remove?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if(result == QMessageBox::Yes)
    {
        qDebug() << "remove more stuff";
        rmv_dialog = new RemoveDialog(this);
        connect(rmv_dialog, SIGNAL(get_person_finished(QString)), this, SLOT(remove_one(QString)), Qt::QueuedConnection);
        connect(db, SIGNAL(delete_success(int)), recognizer_client, SLOT(update_info(int)), Qt::QueuedConnection);
        int state = ins_dialog->exec();
        if(state == QDialog::Accepted)
        {
            recognizer_client->switch_mode(Recognizer::REMOVE);
        }
        else
        {
            recognizer_client->switch_mode(Recognizer::DETECT);
        }
    }
    else if(result == QMessageBox::No)
    {
        qDebug() << "exit remove mode";
        ui->detect_btn->setChecked(true);
        recognizer_client->switch_mode(Recognizer::DETECT);
    }
    else
    {
        qDebug() << "exit remove mode";
        ui->detect_btn->setChecked(true);
        recognizer_client->switch_mode(Recognizer::DETECT);
    }
}

void MainWindow::insert_new(util::person_info info)
{
    db->insert_new_person(info);
}

void MainWindow::get_face_label(int number)
{
    db->lookup_info(number);
}

void MainWindow::remove_one(QString id)
{
    db->delete_person(id);
}

void MainWindow::draw(QImage img)
{
    ui->show->setPixmap(QPixmap::fromImage(img.scaled(ui->show->size(),Qt::KeepAspectRatio)));
}

void MainWindow::on_collect_btn_clicked()
{
    ins_dialog = new InsertDialog(this);
    qRegisterMetaType<util::person_info>("util::person_info");
    // first update database after dialog closed
    // second update training target in recognizer
    connect(ins_dialog, SIGNAL(get_person_finished(util::person_info)), this, SLOT(insert_new(util::person_info)), Qt::QueuedConnection);
    connect(db, SIGNAL(update_success(util::person_info)), recognizer_client, SLOT(update_info(util::person_info)), Qt::QueuedConnection);
    int state = ins_dialog->exec();
    if(state == QDialog::Accepted)
    {
        recognizer_client->switch_mode(Recognizer::GET_FACE_DATA);
    }
    else
    {
        recognizer_client->switch_mode(Recognizer::DETECT);
    }
}

void MainWindow::on_remove_btn_clicked()
{
    rmv_dialog = new RemoveDialog(this);
    connect(rmv_dialog, SIGNAL(get_person_finished(QString)), this, SLOT(remove_one(QString)), Qt::QueuedConnection);
    connect(db, SIGNAL(delete_success(int)), recognizer_client, SLOT(update_info(int)), Qt::QueuedConnection);
    int state = rmv_dialog->exec();
    if(state == QDialog::Accepted)
    {
        recognizer_client->switch_mode(Recognizer::REMOVE);
    }
    else
    {
        recognizer_client->switch_mode(Recognizer::DETECT);
    }
}

void MainWindow::on_detect_btn_clicked()
{
    recognizer_client->switch_mode(Recognizer::DETECT);
}

void MainWindow::on_recognize_btn_clicked()
{
    recognizer_client->switch_mode(Recognizer::RECOGNIZE);
}

