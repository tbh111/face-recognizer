#ifndef DATABASE_H
#define DATABASE_H
#include <QObject>
#include <QtSql>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QDebug>
#include "utils.h"

class Database:public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr);
    void connect_database();
    void close_database();
    int lookup_total_numbers(); // total number in the database
    int insert_new_person(util::person_info info); // insert a new person into the database
    int delete_person(QString id);
public slots:
    int lookup_info(int number);
signals:
    void connected();
    void update_success(util::person_info info);
    void delete_success(int number);
    void lookup_suceess(util::person_info info);
private:
    QSqlDatabase db;
};

#endif // DATABASE_H
