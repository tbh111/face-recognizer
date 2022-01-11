#include "database.h"
Database::Database(QObject *parent):
    QObject(parent)
{
//    database format:
//    +------------+-------------+------+-----+---------+----------------+
//    | Field      | Type        | Null | Key | Default | Extra          |
//    +------------+-------------+------+-----+---------+----------------+
//    | number     | int(11)     | NO   | PRI | NULL    | auto_increment |
//    | id         | char(6)     | NO   |     | NULL    |                |
//    | name       | varchar(10) | NO   |     | NULL    |                |
//    | permission | int(11)     | NO   |     | NULL    |                |
//    | last_login | datetime    | NO   |     | NULL    |                |
//    +------------+-------------+------+-----+---------+----------------+

    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("face_data");
    db.setUserName("admin"); // or root
    db.setPassword("face_admin"); // or panasonic
}

void Database::connect_database()
{
    if(!db.open())
    {
        qDebug() << "failed to open database, try to restart sql service";
        QProcess::execute("'123456' | sudo -S service mysql start");
    }
    else
    {
        qDebug() << "open database success";
        emit connected();
    }
}

void Database::close_database()
{
    db.close();
}

int Database::lookup_total_numbers()
{
    int count = 0;
    QSqlQuery query_count = db.exec("select count(*) from face_data1");
    while(query_count.next())
    {
        count = query_count.value("count(*)").toInt();
        qDebug() << "total person: " << count;
    }
    return count;
}

int Database::lookup_info(int number)
{
    util::person_info lookup_person(number);
    QString cmd = QString("select * from face_data1 where number=%1").arg(number);
//    qDebug() << cmd;
    QSqlQuery query_lookup_info = db.exec(cmd);
    while(query_lookup_info.next())
    {
        qDebug() << "select stuff into database"
                 << "number: " << query_lookup_info.value("number").toInt()
                 << "id" << query_lookup_info.value("id").toString()
                 << "name" << query_lookup_info.value("name").toString()
                 << "permission" << query_lookup_info.value("permission").toInt()
                 << "last_login" << query_lookup_info.value("last_login").toString();
        lookup_person.number_info = query_lookup_info.value("number").toInt();
        lookup_person.id_info = query_lookup_info.value("id").toString();
        lookup_person.name_info = query_lookup_info.value("name").toString();
        lookup_person.permission_info = query_lookup_info.value("permission").toInt();
        lookup_person.last_login_info = query_lookup_info.value("last_login").toString();
        emit lookup_suceess(lookup_person);
    }
    cmd = QString("update face_data1 set last_login=now() where number=%1").arg(lookup_person.number_info);
    return query_lookup_info.size();
}

int Database::insert_new_person(util::person_info info)
{
    int number = -1;
    QString id = "'" + info.id_info + "'";
    QString name = "'" + info.name_info + "'";
    int permission = info.permission_info;
    QString cmd = QString("insert into face_data1(id, name, permission, last_login) values(%1, %2, %3, now())").arg(id).arg(name).arg(permission);
    QSqlQuery query_insert = db.exec(cmd);
    QString cmd_update = QString("select number from face_data1 where id=%1").arg(id);
    QSqlQuery query_update = db.exec(cmd_update);
    while(query_update.next())
    {
        number = query_update.value("number").toInt();
    }
    info.number_info = number;
    emit update_success(info);
    return query_insert.numRowsAffected();
}

int Database::delete_person(QString id)
{
    int number = -1;
    id = "'" + id + "'";
    QString cmd = QString("select number from face_data1 where id=%1").arg(id);
    QSqlQuery query_id = db.exec(cmd);
    while(query_id.next())
    {
        number = query_id.value("number").toInt();
        qDebug() << "delete " << number;
    }
    cmd = QString("delete from face_data1 where number=%1").arg(number);
    QSqlQuery query_delete = db.exec(cmd);
    emit delete_success(number);
    return query_delete.numRowsAffected();
}
