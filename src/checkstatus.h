#ifndef CHECKSTATUS_H
#define CHECKSTATUS_H

#include <QNetworkReply>
#include <QObject>
#include <QTimer>

class CheckStatus : public QObject
{
    Q_OBJECT
private:
	QString m_url;
	QNetworkAccessManager *m_manager;
    bool m_check;
	bool m_dbOpen;
	QTimer *m_timer;

public:
    explicit CheckStatus(QObject *parent = 0);
	~CheckStatus();

signals:

public slots:
	void init();
	void setState(QNetworkReply *reply);
	void check();
};

#endif // CHECKSTATUS_H
