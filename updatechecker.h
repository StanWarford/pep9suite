#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QObject>
#include <QRunnable>
class UpdateChecker : public QObject
{
    Q_OBJECT
public:
    explicit UpdateChecker(QObject *parent=nullptr);
    virtual ~UpdateChecker();
signals:
    void updateInformation(int key);
public slots:
    void beginUpdateCheck();
};

#endif // UPDATECHECKER_H
