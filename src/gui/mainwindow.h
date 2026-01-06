#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QSplitter>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QProgressBar>
#include <QThread>

class FTPWorker;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnect();
    void onDisconnect();
    void onRefreshLocal();
    void onRefreshRemote();
    void onUpload();
    void onDownload();
    void onLocalItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onRemoteItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onDeleteLocal();
    void onDeleteRemote();
    void onCreateLocalFolder();
    void onCreateRemoteFolder();
    void onRenameLocal();
    void onRenameRemote();
    void onConnectionEstablished(const QString &message);
    void onConnectionFailed(const QString &error);
    void onRemoteListReceived(const QStringList &items);
    void onFileTransferProgress(qint64 current, qint64 total);
    void onFileTransferComplete(const QString &message);
    void onFileTransferError(const QString &error);
    void onStatusMessage(const QString &message);

private:
    void setupUI();
    void createActions();
    void createToolBar();
    void createStatusBar();
    void setupLocalBrowser();
    void setupRemoteBrowser();
    void loadLocalDirectory(const QString &path);
    void loadRemoteDirectory(const QString &path);
    QString formatFileSize(qint64 size);

    // UI Components
    QSplitter *mainSplitter;
    QTreeWidget *localBrowser;
    QTreeWidget *remoteBrowser;
    QToolBar *toolBar;
    QLabel *statusLabel;
    QLabel *connectionLabel;
    QProgressBar *progressBar;

    // Actions
    QAction *connectAction;
    QAction *disconnectAction;
    QAction *uploadAction;
    QAction *downloadAction;
    QAction *refreshLocalAction;
    QAction *refreshRemoteAction;
    QAction *createLocalFolderAction;
    QAction *createRemoteFolderAction;
    QAction *deleteLocalAction;
    QAction *deleteRemoteAction;
    QAction *renameLocalAction;
    QAction *renameRemoteAction;

    // FTP Client
    QString currentLocalPath;
    QString currentRemotePath;
    QString ftpHost;
    int ftpPort;
    QString ftpUsername;
    bool isConnected;

    // Worker thread for FTP operations
    QThread *ftpThread;
    FTPWorker *ftpWorker;
};
#endif // MAINWINDOW_H
