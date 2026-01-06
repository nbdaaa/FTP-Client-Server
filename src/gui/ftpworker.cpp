#include "ftpworker.h"

#include <QFileInfo>

#include "../core/ftp_client.h"
#include "../utils/ftp_listing_parser.h"

FTPWorker::FTPWorker(QObject *parent)
    : QObject(parent)
    , ftpClient(nullptr)
    , connected(false)
{
}

FTPWorker::~FTPWorker()
{
    if (ftpClient) {
        delete ftpClient;
    }
}

void FTPWorker::connectToServer(const QString &host, int port,
                                const QString &username, const QString &password)
{
    try {
        ftpClient = new FTPClient(host.toStdString(), port);

        if (ftpClient->connect()) {
            if (ftpClient->login(username.toStdString(), password.toStdString())) {
                connected = true;
                emit connectionEstablished("Connected successfully");
            } else {
                delete ftpClient;
                ftpClient = nullptr;
                emit connectionFailed("Login failed");
            }
        } else {
            delete ftpClient;
            ftpClient = nullptr;
            emit connectionFailed("Failed to connect to server");
        }
    } catch (const std::exception &e) {
        if (ftpClient) {
            delete ftpClient;
            ftpClient = nullptr;
        }
        emit connectionFailed(QString("Error: %1").arg(e.what()));
    }
}

void FTPWorker::disconnectFromServer()
{
    if (ftpClient) {
        try {
            // Send QUIT command if connected
            if (connected) {
                ftpClient->quit();
            }
        } catch (...) {
            // Ignore exceptions during disconnect
        }
        delete ftpClient;
        ftpClient = nullptr;
    }
    connected = false;
}

void FTPWorker::changeRemoteDirectory(const QString &path)
{
    if (!connected || !ftpClient) {
        return;
    }

    try {
        if (ftpClient->changeDirectory(path.toStdString())) {
            emit statusMessage("Changed directory to: " + path);
            // Automatically list the new directory
            listRemoteDirectory(path);
        } else {
            emit fileTransferError("Failed to change directory to: " + path);
        }
    } catch (const std::exception &e) {
        emit fileTransferError(QString("Error changing directory: %1").arg(e.what()));
    }
}

void FTPWorker::listRemoteDirectory(const QString &path)
{
    Q_UNUSED(path);  // We list the current server directory

    if (!connected || !ftpClient) {
        return;
    }

    try {
        std::string listing = ftpClient->list();
        
        // Use FTPListingParser to parse the listing
        QStringList items = FTPListingParser::parseListing(listing);
        
        emit remoteListReceived(items);
    } catch (const std::exception &e) {
        emit statusMessage(QString("Error listing directory: %1").arg(e.what()));
    }
}

void FTPWorker::uploadFile(const QString &localPath, const QString &remotePath)
{
    if (!connected || !ftpClient) {
        return;
    }

    try {
        emit statusMessage("Uploading " + QFileInfo(localPath).fileName() + "...");

        if (ftpClient->put(localPath.toStdString(), remotePath.toStdString())) {
            emit fileTransferComplete("Upload completed: " + QFileInfo(localPath).fileName());
        } else {
            emit fileTransferError("Upload failed: " + QFileInfo(localPath).fileName());
        }
    } catch (const std::exception &e) {
        emit fileTransferError(QString("Upload error: %1").arg(e.what()));
    }
}

void FTPWorker::downloadFile(const QString &remotePath, const QString &localPath)
{
    if (!connected || !ftpClient) {
        return;
    }

    try {
        emit statusMessage("Downloading " + QFileInfo(localPath).fileName() + "...");

        if (ftpClient->get(remotePath.toStdString(), localPath.toStdString())) {
            emit fileTransferComplete("Download completed: " + QFileInfo(localPath).fileName());
        } else {
            emit fileTransferError("Download failed: " + QFileInfo(localPath).fileName());
        }
    } catch (const std::exception &e) {
        emit fileTransferError(QString("Download error: %1").arg(e.what()));
    }
}

void FTPWorker::deleteRemoteFile(const QString &path)
{
    if (!connected || !ftpClient) {
        return;
    }

    emit statusMessage("Delete operation not yet implemented in FTP client");
    Q_UNUSED(path);
}

void FTPWorker::createRemoteDirectory(const QString &path)
{
    if (!connected || !ftpClient) {
        return;
    }

    try {
        emit statusMessage("Creating directory: " + path);

        if (ftpClient->mkdir(path.toStdString())) {
            emit fileTransferComplete("Directory created: " + path);
            // reload listing to reflect the new folder
            listRemoteDirectory(path);
        } else {
            emit fileTransferError("Failed to create directory: " + path);
        }
    } catch (const std::exception &e) {
        emit fileTransferError(QString("Error creating directory: %1").arg(e.what()));
    }
}

void FTPWorker::renameRemoteFile(const QString &oldPath, const QString &newPath)
{
    if (!connected || !ftpClient) {
        return;
    }

    emit statusMessage("Rename operation not yet implemented in FTP client");
    Q_UNUSED(oldPath);
    Q_UNUSED(newPath);
}


