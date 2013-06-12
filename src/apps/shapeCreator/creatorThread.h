#ifndef CREATORTHREAD_H
#define CREATORTHREAD_H

#include <QThread>

class CreatorThread : public QThread
{
    Q_OBJECT
public:
    explicit CreatorThread(QObject *parent = 0);
    
	void run();

signals:
	void onTestStart();
    void onTestProgress(float percent);
    void onTestEnd();
    void onTestMsg(QString msg);
    
public slots:
    
};

#endif // CREATORTHREAD_H
