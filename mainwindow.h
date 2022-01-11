#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include <QTimer>
#include <QImage>
#include <ctime>
#include <QCloseEvent>
#include <QMutex>
#include <QButtonGroup>
#include <QDialog>
#include <QMetaType>
#include "camera.h"
#include "recognizer.h"
#include "database.h"
#include "insertdialog.h"
#include "removedialog.h"
#include "utils.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void closeEvent(QCloseEvent *event);
    ~MainWindow();

private slots:
//    void draw_src(QImage img);
    void draw(QImage img);
    void print_info(util::person_info info);

    // database interface
    void insert_new(util::person_info info);
    void remove_one(QString id);
    void get_face_label(int number);

    // more info or not
    void train_one_end();
    void remove_one_end();

    void on_collect_btn_clicked();
    void on_detect_btn_clicked();
    void on_recognize_btn_clicked();
    void on_remove_btn_clicked();

private:
    Ui::MainWindow *ui;
    InsertDialog *ins_dialog;
    RemoveDialog *rmv_dialog;
    Camera* cam;
    Recognizer* recognizer_client;
    Database* db;
    QMutex mutex;
};

#endif // MAINWINDOW_H
