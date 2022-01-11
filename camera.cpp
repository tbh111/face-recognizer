#include "camera.h"

Camera::Camera(int index, QObject *parent):
    QThread(parent)
{
    cap = VideoCapture(index);
    cap.set(CAP_PROP_FRAME_HEIGHT, 240);
    cap.set(CAP_PROP_FRAME_WIDTH, 320);
}

void Camera::run()
{
    while(true)
    {
        if(!camera_enable)
        {

            qDebug() << "camera thread end";
//            camera_stop();
            QThread::msleep(100);
//            exec();
            break;
        }
        mutex.lock();
        cap >> image;
        cvtColor(image, image_show, COLOR_BGR2RGB);
        qimg = QImage(image_show.data, image_show.cols, image_show.rows, static_cast<int>(image_show.step), QImage::Format_RGB888);
        emit q_captured(qimg);
        emit img_captured(image);
        image = Mat::zeros(image.rows, image.cols, CV_8UC3);
        mutex.unlock();
    }
}

Camera::~Camera()
{
    cap.release();
    qDebug() << "camera thread stop";
    if(mutex.tryLock(200))
    {
        mutex.unlock();
    }
}

//void Camera::camera_stop()
//{
//    cap.release();
//    qDebug() <<"camera stop";
//}
