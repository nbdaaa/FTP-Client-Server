#ifndef FTPWORKER_H
#define FTPWORKER_H

#include <QObject>
#include <QStringList>

class FTPClient;

// Worker class to handle FTP operations in a separate thread
class FTPWorker : public QObject
{
    Q_OBJECT

public:
    explicit FTPWorker(QObject *parent = nullptr);
    ~FTPWorker();

public slots:
    void connectToServer(const QString &host, int port,
                         const QString &username, const QString &password);
    void disconnectFromServer();
    void changeRemoteDirectory(const QString &path);
    void listRemoteDirectory(const QString &path);
    void uploadFile(const QString &localPath, const QString &remotePath);
    void downloadFile(const QString &remotePath, const QString &localPath);
    void createRemoteDirectory(const QString &path);

signals:
    void connectionEstablished(const QString &message);
    void connectionFailed(const QString &error);
    void remoteListReceived(const QStringList &items);
    void fileTransferProgress(qint64 current, qint64 total);
    void fileTransferComplete(const QString &message);
    void fileTransferError(const QString &error);
    void statusMessage(const QString &message);

private:
    FTPClient *ftpClient;
    bool connected;
};

#endif // FTPWORKER_H


