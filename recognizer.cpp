#include "recognizer.h"

Recognizer::Recognizer(int init_mode, int init_detect_mode, QObject *parent) :
    QThread(parent)
{
    if(init_mode == GET_FACE_DATA)
    {
        mode = GET_FACE_DATA;
        qDebug() << "getting face data";
    }
    else if(init_mode == DETECT)
    {
        mode = DETECT;
        qDebug() << "detect faces";
    }
    else if(init_mode == RECOGNIZE)
    {
        mode = RECOGNIZE;
        qDebug() << "recognize faces";
    }
    else
    {
        mode = DETECT;
        qDebug() << "detect faces";
    }
    if(init_detect_mode == RESNET)
    {
        detect_mode = RESNET;
        qDebug() << "using Resnet model to detect faces";
    }
    else if(init_detect_mode == HAAR_CASCADE)
    {
        detect_mode = HAAR_CASCADE;
        qDebug() << "using haar cascade model to detect faces";
    }
    else
    {
        detect_mode = LBP_CASCADE;
        qDebug() << "using LBP cascade model to detect faces";
    }
}

Recognizer::~Recognizer()
{
    qDebug() << "recognize thread stop";
    if(mutex.tryLock(200))
    {
        mutex.unlock();
    }
}

void Recognizer::run()
{
    init();
    while(true)
    {
        mutex.lock();
        if(!recognizer_enable)
        {
            qDebug() << "recognize thread end";
            QThread::msleep(100);
            break;
//            exec();
        }
        else if(image_update_flag)
        {
            process();
        }
        image_update_flag = false;
        mutex.unlock();
    }

}

void Recognizer::init()
{
    if(detect_mode == LBP_CASCADE)
    {
        face_detector.load("../model/lbpcascade_frontalface_improved.xml");
        qDebug() << "successfully load LBP cascade detector model";
    }
    else if(detect_mode == HAAR_CASCADE)
    {
        face_detector.load("../model/haarcascade_frontalface_default.xml");
        qDebug() << "successfully load HAAR cascade detector model";
    }
    else if(detect_mode == RESNET)
    {
        QString model_config = "../model/deploy.prototxt";
        QString model_binary = "../model/res10_300x300_ssd_iter_140000.caffemodel";
        net = readNetFromCaffe(model_config.toStdString(), model_binary.toStdString());
        if(!net.empty())
        {
            qDebug() << "successfully load RESNET detector model";
        }
    }
    else
    {
        face_detector.load("../model/lbpcascade_frontalface_improved.xml");
        qDebug() << "successfully load default LBP cascade detector model";
    }

    model = LBPHFaceRecognizer::create(1,8,8,8,115.0);
    QFile model_path("../model/faceLBPHModel.xml");
    if(model_path.exists())
    {
        model->read("../model/faceLBPHModel.xml");
        qDebug() << "successfully load recognizer model";
        recognizer_model_exist = true;
    }
    else
    {
        qDebug() << "model empty";
        recognizer_model_exist = false;
    }

}

void Recognizer::process()
{
    QImage processed_im;
    if(mode == TRAIN)
    {
        QString image_text = "training...";

        QImage image(show_image.data, show_image.cols, show_image.rows, static_cast<int>(show_image.step), QImage::Format_RGB888);
        QPainter painter(&image);
        QPen pen = QPen(Qt::red, 5);
        QBrush brush = QBrush(Qt::red);
        painter.setPen(pen);
        painter.setBrush(brush);
        painter.drawText(QPointF(20,30), image_text);
        processed_im = image;
        emit process_image_finished(processed_im);
        train_configure();
        train_model();
    }
    else if(mode == REMOVE)
    {
        QString image_text = "removing...";
        QImage image(show_image.data, show_image.cols, show_image.rows, static_cast<int>(show_image.step), QImage::Format_RGB888);
        QPainter painter(&image);
        QPen pen = QPen(Qt::red, 5);
        QBrush brush = QBrush(Qt::red);
        painter.setPen(pen);
        painter.setBrush(brush);
        painter.drawText(QPointF(20,30), image_text);
        processed_im = image;
        emit process_image_finished(processed_im);
        remove_configure();
        remove_train_model();

    }
    else
    {
        double t = static_cast<double>(getTickCount());
        if(mode == GET_FACE_DATA)
        {
            processed_im = get_face_data();
        }
        else if(mode == DETECT)
        {
            processed_im = detect();
        }
        else if(mode == RECOGNIZE)
        {
            processed_im = recognize();
        }
        else
        {
            processed_im = detect();
        }
        t = (static_cast<double>(getTickCount() - t)) / getTickFrequency();
        double fps = 1.0 / t;
        emit send_fps(fps);
        emit process_image_finished(processed_im);
    }
}


QImage Recognizer::detect()
{
    if(detect_mode == RESNET)
    {
        const Scalar mean_val(104.0, 177.0, 123.0);
        const size_t in_width = 300;
        const size_t in_height = 300;
        const double in_scale_vector = 1.0;
        float min_confidence = 0.5;
        Mat input_blob = blobFromImage(src_image, in_scale_vector, Size(in_width, in_height), mean_val, false, false);
        net.setInput(input_blob, "data");
        Mat detection = net.forward("detection_out");
        Mat detection_mat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());
        for(int i=0; i<detection_mat.rows; i++)
        {
            float confidence = detection_mat.at<float>(i,2);
            if(confidence > min_confidence)
            {
                int x_left_bottom = static_cast<int>(detection_mat.at<float>(i,3)*src_image.cols);
                int y_left_bottom = static_cast<int>(detection_mat.at<float>(i,4)*src_image.rows);
                int x_right_top = static_cast<int>(detection_mat.at<float>(i,5)*src_image.cols);
                int y_right_top = static_cast<int>(detection_mat.at<float>(i,6)*src_image.rows);
                Rect face(static_cast<int>(x_left_bottom), static_cast<int>(y_left_bottom),
                          static_cast<int>(x_right_top-x_left_bottom), static_cast<int>(y_right_top-y_left_bottom));
                rectangle(show_image, face, Scalar(0,255,0), 2, 8);
            }
        }
    }
    else
    {
        std::vector<Rect> faces;
        cvtColor(src_image, gray_image, COLOR_BGR2GRAY);
        equalizeHist(gray_image, gray_image);
        face_detector.detectMultiScale(gray_image, faces);
        for(uint i=0; i < faces.size(); i++)
        {
//            qDebug() << "find " << faces.size() << " faces";
            Point pt1 = Point2d(faces[i].x, faces[i].y);
            Point pt2 = Point2d(faces[i].x+faces[i].width, faces[i].y+faces[i].height);
            rectangle(show_image, pt1, pt2, Scalar(0,255,0), 2, 8);
        }
    }
    cvtColor(show_image, show_image, COLOR_BGR2RGB);
    QImage image(show_image.data, show_image.cols, show_image.rows, static_cast<int>(show_image.step), QImage::Format_RGB888);
    frame_number++;
//    qDebug() << "send frame" << frame_number;
    return image;
}

QImage Recognizer::get_face_data()
{
    std::vector<Rect> faces;
    cvtColor(src_image, gray_image, COLOR_BGR2GRAY);
    equalizeHist(gray_image, gray_image);
    face_detector.detectMultiScale(gray_image, faces);
    if(faces.size() > 1)
    {
        qDebug() << "find multiple faces";
    }
    else if(faces.size() == 0)
    {
        qDebug() << "no face detected";
    }
    else
    {
        qDebug() << "saving your face";
        Point pt1 = Point2d(faces[0].x, faces[0].y);
        Point pt2 = Point2d(faces[0].x+faces[0].width, faces[0].y+faces[0].height);
        Rect face_rect(faces[0].x, faces[0].y, faces[0].width, faces[0].height);
        Mat face_roi = show_image(face_rect);
        QString path = "/home/pi/face/datasets/" + QString::number(current_info.number_info) + "/";
        QDir dir(path);
        if(!dir.exists())
        {
            dir.mkdir(path);
        }
        if(++collected_faces == 20)
        {
            collected_person ++;
            qDebug() << "finished collecting " << current_info.number_info << " persons";
            emit get_one_face_data_finished();
//            train_configure();
            switch_mode(TRAIN);
            collected_faces = 0;
        }
//        if(collected_person == target_person)
//        {
//            qDebug() << "finished collecting all person";
//            emit get_all_face_data_finished();
//            train_configure();
//            switch_mode(TRAINING);
//        }
        else
        {
            QString file = path + QString::number(frame_number, 10) + ".png";
            imwrite(file.toStdString(), face_roi);
            qDebug() << "save: " << file;
        }
        rectangle(show_image, pt1, pt2, Scalar(0,255,0), 2, 8);
    }
    cvtColor(show_image, show_image, COLOR_BGR2RGB);
    QImage image(show_image.data, show_image.cols, show_image.rows, static_cast<int>(show_image.step), QImage::Format_RGB888);

    frame_number++;
//    qDebug() << "send frame" << frame_number;

    return image;
}

QImage Recognizer::recognize()
{
    std::vector<Rect> faces;
    Mat face_roi;
    int predict_label = -1;
    double confidence = 0.0;
    cvtColor(src_image, gray_image, COLOR_BGR2GRAY);
    equalizeHist(gray_image, gray_image);
    face_detector.detectMultiScale(gray_image, faces);
    for(uint i=0; i < faces.size(); i++)
    {
//        qDebug() << "find " << faces.size() << " faces";
        Point pt1 = Point2d(faces[i].x, faces[i].y);
        Point pt2 = Point2d(faces[i].x+faces[i].width, faces[i].y+faces[i].height);
        Rect face_rect(faces[0].x, faces[0].y, faces[0].width, faces[0].height);
        Mat face_roi = show_image(face_rect);
        cvtColor(face_roi, face_roi, COLOR_BGR2GRAY);
        resize(face_roi, face_roi, Size(static_cast<int>(face_roi.cols/2), static_cast<int>(face_roi.rows/2)), 0, 0, INTER_AREA);
        if(recognizer_model_exist)
        {
            model->predict(face_roi, predict_label, confidence);
            qDebug() << predict_label << " " << confidence;
            rectangle(show_image, pt1, pt2, Scalar(0,255,0), 2, 8);
            if(confidence < 115)
            {
                emit face_pass(predict_label);
            }
//            putText(show_image, QString::number(predict_label).toStdString(), Point(pt1.x, pt1.y+3),
//                    FONT_HERSHEY_SIMPLEX, 1.5, Scalar(255,0,0),3);
        }
//        qDebug() << "confidence " << confidence;
    }
    cvtColor(show_image, show_image, COLOR_BGR2RGB);
    QImage image(show_image.data, show_image.cols, show_image.rows, static_cast<int>(show_image.step), QImage::Format_RGB888);

    frame_number++;
//    qDebug() << "send frame" << frame_number;
    return image;
}

void Recognizer::train_configure()
{
    QFile config_file("/home/pi/face/datasets/config.txt");
    if(!config_file.exists())
    {
        qDebug() << "config file not exist, create it";
    }
    if(!config_file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append))
    {
        qDebug() << "can not open config file";
    }

    QTextStream out(&config_file);

    QString path = "/home/pi/face/datasets/" + QString::number(current_info.number_info) + "/";
    QDir dir(path);
    if(!dir.exists())
    {
        qDebug() << "can not find directory:" << path;
    }
    QStringList filters, file_list;
    QString filename, filepath;
    filters << QString("*.png");
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    dir.setNameFilters(filters);
    qDebug() << "find " << dir.count() << " png files";
    for(uint j=0; j<dir.count(); j++)
    {
        filename = dir[int(j)];
        filepath = path + filename;
        out << filepath << ";" << QString::number(current_info.number_info) << "\n";
    }

    config_file.close();
    qDebug() << "finished processing config file";
}

void Recognizer::remove_configure()
{
    QFile config_file("/home/pi/face/datasets/config.txt");
    if(!config_file.exists())
    {
        qDebug() << "config file not exist, remove failed";
        emit remove_face_data_failed();
        return;
    }
    if(!config_file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qDebug() << "can not open config file";
        emit remove_face_data_failed();
        return;
    }
    QTextStream config_in(&config_file);
    QString str_all = config_in.readAll();
    config_file.close();

    QString source_path = "/home/pi/face/datasets/" + QString::number(current_info.number_info) + "/";
    QDir dir(source_path);
    if(!dir.exists())
    {
        qDebug() << "can not find directory:" << source_path;
    }
    int image_count = static_cast<int>(dir.count());
    qDebug() << "find" << image_count << "png files";
    if(dir.removeRecursively())
    {
        qDebug() << "removed " << image_count << " png files";
    }

    if(!config_file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
    {
        qDebug() << "can not reopen config file";
        emit remove_face_data_failed();
        return;
    }
    QTextStream config_out(&config_file);
    QStringList str_list = str_all.split("\n");
    QString remove_number = ";"+QString::number(current_info.number_info);

    for(int i=0; i < str_list.count(); i++)
    {
        if(str_list.at(i).indexOf(remove_number, -remove_number.length()) < 0)
        {
            config_out << str_list.at(i) << "\n";
        }
    }
    config_file.close();
    qDebug() << "finished processing config file";
}

void Recognizer::read_txt(std::vector<Mat> &images, std::vector<int> &labels, QString separator)
{
    QFile config_file("/home/pi/face/datasets/config.txt");
//    qDebug() << "current number" << current_info.number_info;
    if(!config_file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "can not open train config file";
    }
    QTextStream read_stream(&config_file);
    QString line_in;
    read_stream.seek(0);
    while(!read_stream.atEnd())
    {
        line_in = read_stream.readLine();
        QStringList list = line_in.split(separator);
        if(mode == REMOVE)
        {
            if(!list[0].isEmpty() && !list[1].isEmpty() && (list[1].toInt() != current_info.number_info))
            {
                qDebug() << "read: " << list[0];
                Mat img = imread(list[0].toStdString(), 0);
                resize(img, img, Size(static_cast<int>(img.cols/2), static_cast<int>(img.rows/2)), 0, 0, INTER_AREA);
                equalizeHist(img, img);
                images.push_back(img);
                labels.push_back(list[1].toInt());
            }
        }
        else
        {
            if(!list[0].isEmpty() && !list[1].isEmpty() && (list[1].toInt() == current_info.number_info))
            {
                qDebug() << "read: " << list[0];
                Mat img = imread(list[0].toStdString(), 0);
                resize(img, img, Size(static_cast<int>(img.cols/2), static_cast<int>(img.rows/2)), 0, 0, INTER_AREA);
                equalizeHist(img, img);
                images.push_back(img);
                labels.push_back(list[1].toInt());
            }
        }
    }
    config_file.close();
}

void Recognizer::train_model()
{
    std::vector<Mat> images;
    std::vector<int> labels;
    read_txt(images, labels, ";");
    if(!recognizer_model_exist)
    {
        qDebug() << "training";
        model->train(images, labels);
        recognizer_model_exist = true;
    }
    else
    {
        qDebug() << "update recognize model: "
                 << "number: " << current_info.number_info
                 << "id" << current_info.id_info
                 << "name" << current_info.name_info
                 << "permission" << current_info.permission_info;
        model->update(images, labels);
    }
    model->save("../model/faceLBPHModel.xml");
    model->clear();
    model = LBPHFaceRecognizer::create(1,8,8,8,115.0);
    model->read("../model/faceLBPHModel.xml");
    qDebug() << "finished training LBPH model";
    emit end_training();
    images.clear();
    labels.clear();
    switch_mode(DETECT);
}

void Recognizer::remove_train_model()
{
    std::vector<Mat> images;
    std::vector<int> labels;
    read_txt(images, labels, ";");
    if(!recognizer_model_exist)
    {
        qDebug() << "model empty, remove failed";
        switch_mode(DETECT);
    }
    else
    {
        model->clear();
        model->train(images, labels);
        model->save("../model/faceLBPHModel.xml");
        model = LBPHFaceRecognizer::create(1,8,8,8,115.0);
        model->read("../model/faceLBPHModel.xml");
        qDebug() << "finished training LBPH model after training";
        emit end_removing();
        images.clear();
        labels.clear();
        switch_mode(DETECT);
    }
}

void Recognizer::update_info(util::person_info new_info)
{
    current_info = new_info;
    qDebug() << "select stuff from database"
             << "number: " << current_info.number_info
             << "id" << current_info.id_info
             << "name" << current_info.name_info
             << "permission" << current_info.permission_info
             << "last_login" << current_info.last_login_info;
}

void Recognizer::update_info(int number)
{
    current_info.number_info = number;
    current_info.id_info = "999999";
    current_info.name_info = "guest";
    current_info.permission_info = 1;
    current_info.last_login_info = "";
    qDebug() << "remove stuff from database"
             << "number:" << number;
}

void Recognizer::switch_mode(int change_mode)
{
    mode = change_mode;
    qDebug() << "switch to" << mode_type_metaenum.valueToKey(change_mode);
}

void Recognizer::get_image(Mat image)
{
    mutex.lock();
//    qDebug() << "get frame";
//    src_image = image.clone();
//    show_image = image.clone();
    image.copyTo(src_image);
    image.copyTo(show_image);
    image_update_flag = true;
    mutex.unlock();
}
