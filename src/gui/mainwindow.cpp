#include "mainwindow.h"
#include "connectiondialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QHeaderView>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QMenu>
#include <QMenuBar>
#include <QRegExp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , isConnected(false)
    , ftpThread(nullptr)
    , ftpWorker(nullptr)
{
    currentLocalPath = QDir::homePath();
    currentRemotePath = "/";

    setupUI();
    createActions();
    createToolBar();
    createStatusBar();

    loadLocalDirectory(currentLocalPath);

    resize(1200, 700);
    setWindowTitle("Cp.FTP - FTP Client");
}

MainWindow::~MainWindow()
{
    if (ftpThread) {
        ftpThread->quit();
        ftpThread->wait();
    }
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    mainSplitter = new QSplitter(Qt::Horizontal, this);

    setupLocalBrowser();
    setupRemoteBrowser();

    mainSplitter->addWidget(localBrowser);
    mainSplitter->addWidget(remoteBrowser);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 1);

    mainLayout->addWidget(mainSplitter);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

void MainWindow::setupLocalBrowser()
{
    localBrowser = new QTreeWidget(this);
    localBrowser->setHeaderLabels(QStringList() << "Name" << "Size" << "Modified");
    localBrowser->setRootIsDecorated(false);
    localBrowser->setAlternatingRowColors(true);
    localBrowser->setSortingEnabled(true);
    localBrowser->setSelectionMode(QAbstractItemView::ExtendedSelection);
    localBrowser->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    localBrowser->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    localBrowser->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    localBrowser->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(localBrowser, &QTreeWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        QMenu contextMenu;
        contextMenu.addAction(uploadAction);
        contextMenu.addAction(refreshLocalAction);
        contextMenu.addSeparator();
        contextMenu.addAction(createLocalFolderAction);
        contextMenu.addAction(renameLocalAction);
        contextMenu.addAction(deleteLocalAction);
        contextMenu.exec(localBrowser->mapToGlobal(pos));
    });

    connect(localBrowser, &QTreeWidget::itemDoubleClicked, this, &MainWindow::onLocalItemDoubleClicked);
}

void MainWindow::setupRemoteBrowser()
{
    remoteBrowser = new QTreeWidget(this);
    remoteBrowser->setHeaderLabels(QStringList() << "Name" << "Size" << "Modified");
    remoteBrowser->setRootIsDecorated(false);
    remoteBrowser->setAlternatingRowColors(true);
    remoteBrowser->setSortingEnabled(true);
    remoteBrowser->setSelectionMode(QAbstractItemView::ExtendedSelection);
    remoteBrowser->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    remoteBrowser->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    remoteBrowser->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    remoteBrowser->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(remoteBrowser, &QTreeWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        QMenu contextMenu;
        contextMenu.addAction(downloadAction);
        contextMenu.addAction(refreshRemoteAction);
        contextMenu.addSeparator();
        contextMenu.addAction(createRemoteFolderAction);
        contextMenu.addAction(renameRemoteAction);
        contextMenu.addAction(deleteRemoteAction);
        contextMenu.exec(remoteBrowser->mapToGlobal(pos));
    });

    connect(remoteBrowser, &QTreeWidget::itemDoubleClicked, this, &MainWindow::onRemoteItemDoubleClicked);
}

void MainWindow::createActions()
{
    connectAction = new QAction(QIcon::fromTheme("network-connect"), "Connect", this);
    connectAction->setShortcut(QKeySequence("Ctrl+N"));
    connect(connectAction, &QAction::triggered, this, &MainWindow::onConnect);

    disconnectAction = new QAction(QIcon::fromTheme("network-disconnect"), "Disconnect", this);
    disconnectAction->setEnabled(false);
    connect(disconnectAction, &QAction::triggered, this, &MainWindow::onDisconnect);

    uploadAction = new QAction(QIcon::fromTheme("upload"), "Upload", this);
    uploadAction->setShortcut(QKeySequence("Ctrl+U"));
    uploadAction->setEnabled(false);
    connect(uploadAction, &QAction::triggered, this, &MainWindow::onUpload);

    downloadAction = new QAction(QIcon::fromTheme("download"), "Download", this);
    downloadAction->setShortcut(QKeySequence("Ctrl+D"));
    downloadAction->setEnabled(false);
    connect(downloadAction, &QAction::triggered, this, &MainWindow::onDownload);

    refreshLocalAction = new QAction(QIcon::fromTheme("view-refresh"), "Refresh Local", this);
    refreshLocalAction->setShortcut(QKeySequence("F5"));
    connect(refreshLocalAction, &QAction::triggered, this, &MainWindow::onRefreshLocal);

    refreshRemoteAction = new QAction(QIcon::fromTheme("view-refresh"), "Refresh Remote", this);
    refreshRemoteAction->setShortcut(QKeySequence("F6"));
    refreshRemoteAction->setEnabled(false);
    connect(refreshRemoteAction, &QAction::triggered, this, &MainWindow::onRefreshRemote);

    createLocalFolderAction = new QAction(QIcon::fromTheme("folder-new"), "New Folder", this);
    connect(createLocalFolderAction, &QAction::triggered, this, &MainWindow::onCreateLocalFolder);

    createRemoteFolderAction = new QAction(QIcon::fromTheme("folder-new"), "New Folder", this);
    createRemoteFolderAction->setEnabled(false);
    connect(createRemoteFolderAction, &QAction::triggered, this, &MainWindow::onCreateRemoteFolder);

    deleteLocalAction = new QAction(QIcon::fromTheme("edit-delete"), "Delete", this);
    connect(deleteLocalAction, &QAction::triggered, this, &MainWindow::onDeleteLocal);

    deleteRemoteAction = new QAction(QIcon::fromTheme("edit-delete"), "Delete", this);
    deleteRemoteAction->setEnabled(false);
    connect(deleteRemoteAction, &QAction::triggered, this, &MainWindow::onDeleteRemote);

    renameLocalAction = new QAction(QIcon::fromTheme("edit-rename"), "Rename", this);
    connect(renameLocalAction, &QAction::triggered, this, &MainWindow::onRenameLocal);

    renameRemoteAction = new QAction(QIcon::fromTheme("edit-rename"), "Rename", this);
    renameRemoteAction->setEnabled(false);
    connect(renameRemoteAction, &QAction::triggered, this, &MainWindow::onRenameRemote);
}

void MainWindow::createToolBar()
{
    toolBar = new QToolBar("Main Toolbar", this);
    toolBar->setMovable(false);
    toolBar->setIconSize(QSize(24, 24));

    toolBar->addAction(connectAction);
    toolBar->addAction(disconnectAction);
    toolBar->addSeparator();
    toolBar->addAction(uploadAction);
    toolBar->addAction(downloadAction);
    toolBar->addSeparator();
    toolBar->addAction(refreshLocalAction);
    toolBar->addAction(refreshRemoteAction);

    addToolBar(toolBar);
}

void MainWindow::createStatusBar()
{
    connectionLabel = new QLabel("Not connected");
    connectionLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    statusLabel = new QLabel("Ready");
    statusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    progressBar = new QProgressBar();
    progressBar->setVisible(false);
    progressBar->setMaximumWidth(200);

    statusBar()->addWidget(connectionLabel, 0);
    statusBar()->addWidget(statusLabel, 1);
    statusBar()->addPermanentWidget(progressBar);
}

void MainWindow::loadLocalDirectory(const QString &path)
{
    localBrowser->clear();
    QDir dir(path);

    if (!dir.exists()) {
        statusLabel->setText("Error: Directory does not exist");
        return;
    }

    QTreeWidgetItem *parentItem = new QTreeWidgetItem(localBrowser);
    parentItem->setText(0, "..");
    parentItem->setIcon(0, QIcon::fromTheme("go-up"));
    parentItem->setData(0, Qt::UserRole, dir.absolutePath() + "/..");

    QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::DirsFirst | QDir::Name);

    for (const QFileInfo &info : entries) {
        QTreeWidgetItem *item = new QTreeWidgetItem(localBrowser);
        item->setText(0, info.fileName());
        item->setData(0, Qt::UserRole, info.absoluteFilePath());

        if (info.isDir()) {
            item->setIcon(0, QIcon::fromTheme("folder"));
            item->setText(1, "<DIR>");
        } else {
            item->setIcon(0, QIcon::fromTheme("text-x-generic"));
            item->setText(1, formatFileSize(info.size()));
        }

        item->setText(2, info.lastModified().toString("yyyy-MM-dd HH:mm"));
    }

    currentLocalPath = dir.absolutePath();
    setWindowTitle(QString("Cp.FTP - Local: %1").arg(currentLocalPath));
}

void MainWindow::loadRemoteDirectory(const QString &path)
{
    Q_UNUSED(path);  // Directory is changed on server side

    if (!isConnected || !ftpWorker) {
        return;
    }

    statusLabel->setText("Loading remote directory...");
    ftpWorker->listRemoteDirectory(currentRemotePath);
}

QString MainWindow::formatFileSize(qint64 size)
{
    const qint64 kb = 1024;
    const qint64 mb = kb * 1024;
    const qint64 gb = mb * 1024;

    if (size >= gb) {
        return QString::number(size / (double)gb, 'f', 2) + " GB";
    } else if (size >= mb) {
        return QString::number(size / (double)mb, 'f', 2) + " MB";
    } else if (size >= kb) {
        return QString::number(size / (double)kb, 'f', 2) + " KB";
    } else {
        return QString::number(size) + " B";
    }
}

void MainWindow::onConnect()
{
    ConnectionDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        ftpHost = dialog.getHost();
        ftpPort = dialog.getPort();
        ftpUsername = dialog.getUsername();
        QString password = dialog.getPassword();

        ftpThread = new QThread(this);
        ftpWorker = new FTPWorker();
        ftpWorker->moveToThread(ftpThread);

        connect(ftpThread, &QThread::finished, ftpWorker, &QObject::deleteLater);
        connect(this, &MainWindow::destroyed, ftpThread, &QThread::quit);

        connect(ftpWorker, &FTPWorker::connectionEstablished, this, &MainWindow::onConnectionEstablished);
        connect(ftpWorker, &FTPWorker::connectionFailed, this, &MainWindow::onConnectionFailed);
        connect(ftpWorker, &FTPWorker::remoteListReceived, this, &MainWindow::onRemoteListReceived);
        connect(ftpWorker, &FTPWorker::fileTransferProgress, this, &MainWindow::onFileTransferProgress);
        connect(ftpWorker, &FTPWorker::fileTransferComplete, this, &MainWindow::onFileTransferComplete);
        connect(ftpWorker, &FTPWorker::fileTransferError, this, &MainWindow::onFileTransferError);
        connect(ftpWorker, &FTPWorker::statusMessage, this, &MainWindow::onStatusMessage);

        ftpThread->start();

        statusLabel->setText("Connecting to server...");
        ftpWorker->connectToServer(ftpHost, ftpPort, ftpUsername, password);
    }
}

void MainWindow::onDisconnect()
{
    if (ftpWorker) {
        ftpWorker->disconnectFromServer();
    }

    if (ftpThread) {
        ftpThread->quit();
        ftpThread->wait();
        ftpThread = nullptr;
    }

    ftpWorker = nullptr;
    isConnected = false;

    remoteBrowser->clear();
    connectionLabel->setText("Not connected");
    statusLabel->setText("Disconnected");

    connectAction->setEnabled(true);
    disconnectAction->setEnabled(false);
    uploadAction->setEnabled(false);
    downloadAction->setEnabled(false);
    refreshRemoteAction->setEnabled(false);
    createRemoteFolderAction->setEnabled(false);
    deleteRemoteAction->setEnabled(false);
    renameRemoteAction->setEnabled(false);
}

void MainWindow::onRefreshLocal()
{
    loadLocalDirectory(currentLocalPath);
    statusLabel->setText("Local directory refreshed");
}

void MainWindow::onRefreshRemote()
{
    loadRemoteDirectory(currentRemotePath);
}

void MainWindow::onUpload()
{
    QList<QTreeWidgetItem*> selectedItems = localBrowser->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "Upload", "Please select files to upload");
        return;
    }

    for (QTreeWidgetItem *item : selectedItems) {
        QString localPath = item->data(0, Qt::UserRole).toString();
        QString fileName = QFileInfo(localPath).fileName();

        if (fileName == "..") {
            continue;
        }

        QFileInfo info(localPath);
        if (info.isFile()) {
            // Use just the filename since we're in the right directory on the server
            statusLabel->setText("Uploading " + fileName + "...");
            ftpWorker->uploadFile(localPath, fileName);
        }
    }
}

void MainWindow::onDownload()
{
    QList<QTreeWidgetItem*> selectedItems = remoteBrowser->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "Download", "Please select files to download");
        return;
    }

    for (QTreeWidgetItem *item : selectedItems) {
        QString fileName = item->text(0);

        if (fileName == ".." || item->text(1) == "<DIR>") {
            continue;
        }

        // Use just the filename since we're in the right directory on the server
        QString localPath = currentLocalPath;
        if (!localPath.endsWith('/') && !localPath.endsWith('\\')) {
            localPath += '/';
        }
        localPath += fileName;

        statusLabel->setText("Downloading " + fileName + "...");
        ftpWorker->downloadFile(fileName, localPath);
    }
}

void MainWindow::onLocalItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    QString path = item->data(0, Qt::UserRole).toString();
    QFileInfo info(path);

    if (info.isDir() || item->text(0) == "..") {
        loadLocalDirectory(QDir(path).absolutePath());
    }
}

void MainWindow::onRemoteItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    QString fileName = item->text(0);

    if (fileName == "..") {
        // Go to parent directory
        ftpWorker->changeRemoteDirectory("..");

        // Update local tracking
        QStringList parts = currentRemotePath.split('/', Qt::SkipEmptyParts);
        if (!parts.isEmpty()) {
            parts.removeLast();
        }
        currentRemotePath = "/" + parts.join('/');
        if (currentRemotePath.isEmpty()) {
            currentRemotePath = "/";
        }
    } else if (item->text(1) == "<DIR>") {
        // Enter directory
        ftpWorker->changeRemoteDirectory(fileName);

        // Update local tracking
        if (!currentRemotePath.endsWith('/')) {
            currentRemotePath += '/';
        }
        currentRemotePath += fileName;
    }
}

void MainWindow::onDeleteLocal()
{
    QList<QTreeWidgetItem*> selectedItems = localBrowser->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete",
        "Are you sure you want to delete the selected items?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        for (QTreeWidgetItem *item : selectedItems) {
            QString path = item->data(0, Qt::UserRole).toString();
            QFileInfo info(path);

            if (info.fileName() == "..") {
                continue;
            }

            if (info.isFile()) {
                QFile::remove(path);
            } else if (info.isDir()) {
                QDir(path).removeRecursively();
            }
        }
        onRefreshLocal();
    }
}

void MainWindow::onDeleteRemote()
{
    QList<QTreeWidgetItem*> selectedItems = remoteBrowser->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete",
        "Are you sure you want to delete the selected items?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        for (QTreeWidgetItem *item : selectedItems) {
            QString fileName = item->text(0);

            if (fileName == "..") {
                continue;
            }

            // Use just the filename since we're in the right directory on the server
            ftpWorker->deleteRemoteFile(fileName);
        }
    }
}

void MainWindow::onCreateLocalFolder()
{
    bool ok;
    QString folderName = QInputDialog::getText(this, "Create Folder",
        "Folder name:", QLineEdit::Normal, "", &ok);

    if (ok && !folderName.isEmpty()) {
        QString path = currentLocalPath;
        if (!path.endsWith('/') && !path.endsWith('\\')) {
            path += '/';
        }
        path += folderName;

        if (QDir().mkdir(path)) {
            onRefreshLocal();
            statusLabel->setText("Folder created: " + folderName);
        } else {
            QMessageBox::warning(this, "Error", "Failed to create folder");
        }
    }
}

void MainWindow::onCreateRemoteFolder()
{
    bool ok;
    QString folderName = QInputDialog::getText(this, "Create Folder",
        "Folder name:", QLineEdit::Normal, "", &ok);

    if (ok && !folderName.isEmpty()) {
        QString newPath = currentRemotePath;
        if (!newPath.endsWith('/'))
            newPath += '/';
        newPath += folderName;

        ftpWorker->createRemoteDirectory(newPath);
        statusLabel->setText("Creating remote folder: " + newPath);
        // refresh view to see the new folder once server responds
        onRefreshRemote();
    }
}

void MainWindow::onRenameLocal()
{
    QTreeWidgetItem *item = localBrowser->currentItem();
    if (!item) {
        return;
    }

    QString oldPath = item->data(0, Qt::UserRole).toString();
    QString oldName = QFileInfo(oldPath).fileName();

    if (oldName == "..") {
        return;
    }

    bool ok;
    QString newName = QInputDialog::getText(this, "Rename",
        "New name:", QLineEdit::Normal, oldName, &ok);

    if (ok && !newName.isEmpty() && newName != oldName) {
        QString newPath = QFileInfo(oldPath).path() + "/" + newName;

        if (QFile::rename(oldPath, newPath)) {
            onRefreshLocal();
            statusLabel->setText("Renamed to: " + newName);
        } else {
            QMessageBox::warning(this, "Error", "Failed to rename");
        }
    }
}

void MainWindow::onRenameRemote()
{
    QTreeWidgetItem *item = remoteBrowser->currentItem();
    if (!item) {
        return;
    }

    QString oldName = item->text(0);

    if (oldName == "..") {
        return;
    }

    bool ok;
    QString newName = QInputDialog::getText(this, "Rename",
        "New name:", QLineEdit::Normal, oldName, &ok);

    if (ok && !newName.isEmpty() && newName != oldName) {
        // Use just the filenames since we're in the right directory on the server
        ftpWorker->renameRemoteFile(oldName, newName);
    }
}

void MainWindow::onConnectionEstablished(const QString &message)
{
    isConnected = true;
    connectionLabel->setText("Connected to " + ftpHost + ":" + QString::number(ftpPort));
    statusLabel->setText(message);

    connectAction->setEnabled(false);
    disconnectAction->setEnabled(true);
    uploadAction->setEnabled(true);
    downloadAction->setEnabled(true);
    refreshRemoteAction->setEnabled(true);
    createRemoteFolderAction->setEnabled(true);
    deleteRemoteAction->setEnabled(true);
    renameRemoteAction->setEnabled(true);

    loadRemoteDirectory(currentRemotePath);
}

void MainWindow::onConnectionFailed(const QString &error)
{
    QMessageBox::critical(this, "Connection Failed", error);
    statusLabel->setText("Connection failed");
    onDisconnect();
}

void MainWindow::onRemoteListReceived(const QStringList &items)
{
    remoteBrowser->clear();

    QTreeWidgetItem *parentItem = new QTreeWidgetItem(remoteBrowser);
    parentItem->setText(0, "..");
    parentItem->setIcon(0, QIcon::fromTheme("go-up"));

    for (const QString &item : items) {
        QStringList parts = item.split('\t');
        if (parts.size() >= 2) {
            QTreeWidgetItem *treeItem = new QTreeWidgetItem(remoteBrowser);
            treeItem->setText(0, parts[0]);

            if (parts[1] == "DIR") {
                treeItem->setIcon(0, QIcon::fromTheme("folder"));
                treeItem->setText(1, "<DIR>");
            } else {
                treeItem->setIcon(0, QIcon::fromTheme("text-x-generic"));
                treeItem->setText(1, parts[1]);
            }

            if (parts.size() >= 3) {
                treeItem->setText(2, parts[2]);
            }
        }
    }

    statusLabel->setText("Remote directory loaded");
}

void MainWindow::onFileTransferProgress(qint64 current, qint64 total)
{
    if (total > 0) {
        progressBar->setVisible(true);
        progressBar->setMaximum(100);
        progressBar->setValue((current * 100) / total);
    }
}

void MainWindow::onFileTransferComplete(const QString &message)
{
    progressBar->setVisible(false);
    statusLabel->setText(message);

    onRefreshLocal();
    onRefreshRemote();
}

void MainWindow::onFileTransferError(const QString &error)
{
    progressBar->setVisible(false);
    QMessageBox::warning(this, "Transfer Error", error);
    statusLabel->setText("Transfer failed");
}

void MainWindow::onStatusMessage(const QString &message)
{
    statusLabel->setText(message);
}

// FTPWorker implementation
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

void FTPWorker::connectToServer(const QString &host, int port, const QString &username, const QString &password)
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
        QStringList items;

        if (listing.empty()) {
            emit remoteListReceived(items);
            return;
        }

        QString qListing = QString::fromStdString(listing);
        QStringList lines = qListing.split('\n', Qt::SkipEmptyParts);

        for (const QString &line : lines) {
            QString trimmed = line.trimmed();
            if (trimmed.isEmpty()) {
                continue;
            }

            // Parse Unix-style directory listing (simplified)
            // Format: drwxr-xr-x 2 user group 4096 Jan 1 12:00 filename
            QStringList parts = trimmed.split(QRegExp("\\s+"), Qt::SkipEmptyParts);

            if (parts.size() >= 9) {
                QString filename = parts.mid(8).join(" ");
                QString permissions = parts[0];
                bool isDir = permissions.startsWith('d');

                QString size = isDir ? "DIR" : parts[4];
                QString date = parts[5] + " " + parts[6] + " " + parts[7];

                items.append(filename + "\t" + size + "\t" + date);
            }
        }

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
}
