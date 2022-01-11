#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QVariant>

namespace util
{
    struct person_info
    {
        int number_info; // primary key
        QString id_info; // stuff id
        QString name_info; // stuff name
        int permission_info; // stuff permission
        QString last_login_info; // last time the stuff log in
        person_info():id_info("999999"), name_info("guest"), permission_info(1){}
        person_info(int number):number_info(number), id_info("999999"), name_info("guest"), permission_info(1){}
        person_info(QString id, QString name, int permission):
          number_info(0), id_info(id), name_info(name), permission_info(permission){}
    };

}

#endif // UTILS_H
