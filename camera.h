#ifndef CAMERA_H
#define CAMERA_H
#include <QMainWindow>
#include <QString>
#include <QThread>
#include <QDebug>
#include <QImage>
#include <QMutex>
#include <opencv2/opencv.hpp>
using namespace cv;

class Camera:public QThread
{
    Q_OBJECT
public:
    explicit Camera(int index, QObject * parent = nullptr);
    ~Camera();
    void run();
    void camera_stop();
    VideoCapture cap;
    bool camera_enable = true;
signals:
    void q_captured(QImage qimg);
    void img_captured(Mat img);
private:
    Mat image, image_show;
    QImage qimg;
    QMutex mutex;
};

#endif // CAMERA_H
