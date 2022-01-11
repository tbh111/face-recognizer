#ifndef RECOGNIZER_H
#define RECOGNIZER_H
#include <QMainWindow>
#include <QString>
#include <QThread>
#include <QDebug>
#include <QImage>
#include <QMutex>
#include <QMetaEnum>
#include <QDir>
#include <QFile>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <opencv2/face/facerec.hpp>
#include "utils.h"
using namespace cv;
using namespace cv::face;
using namespace cv::dnn;

class Recognizer:public QThread
{
    Q_OBJECT
public:
    explicit Recognizer(int init_mode, int detect_mode, QObject * parent = nullptr);
    ~Recognizer();
    void run();
    void init();
    bool recognizer_enable = true;
    bool image_update_flag = false;
    int frame_number = 0;
    enum mode_type {GET_FACE_DATA = 1, DETECT = 2, RECOGNIZE = 3, TRAIN = 4, REMOVE = 5};
    Q_ENUM(mode_type)
    QMetaEnum mode_type_metaenum = QMetaEnum::fromType<mode_type>();
    enum detect_type {LBP_CASCADE = 1, HAAR_CASCADE = 2, RESNET = 3};
public slots:
    void get_image(Mat image);
    void switch_mode(int change_mode);
    void update_info(util::person_info new_info);
    void update_info(int number);
signals:
    void process_image_finished(QImage process_image);
    void get_one_face_data_finished();
    void remove_face_data_failed();
//    void get_all_face_data_finished();
//    void start_training();
    void end_training();
    void end_removing();
    void face_pass(int label);
    void send_fps(double fps);
private:
    void process();
    QImage get_face_data();
    QImage detect();
    QImage recognize();
    void train_configure();
    void remove_configure();
    void read_txt(std::vector<Mat>& images, std::vector<int>& labels, QString separator);
    void remove_train_model();
    void train_model();

    int mode = DETECT;
    int detect_mode = LBP_CASCADE;
    Mat src_image, gray_image, show_image;
    CascadeClassifier face_detector;
    Ptr<FaceRecognizer> model;
    dnn::Net net;
    int collected_faces = 0;
    int collected_person = 0;
//    int target_person = 1;
    bool recognizer_model_exist = false;
    util::person_info current_info;
    QMutex mutex;
};

#endif // RECOGNIZER_H
