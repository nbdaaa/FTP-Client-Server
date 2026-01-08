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
    void onCreateLocalFolder();
    void onCreateRemoteFolder();
    void onRenameLocal();
    void onDeleteRemote();
    void onRenameRemote();
    void onConnectionEstablished(const QString &message);
    void onConnectionFailed(const QString &error);
    void onRemoteListReceived(const QStringList &items);
    void onFileTransferProgress(qint64 current, qint64 total);
    void onFileTransferComplete(const QString &message);
    void onFileTransferError(const QString &error);
    void onStatusMessage(const QString &message);

signals:
    // Signals to request FTP operations (thread-safe communication)
    void requestConnect(const QString &host, int port,
                       const QString &username, const QString &password);
    void requestDisconnect();
    void requestListRemote(const QString &path);
    void requestUpload(const QString &localPath, const QString &remotePath);
    void requestDownload(const QString &remotePath, const QString &localPath);
    void requestChangeDirectory(const QString &path);
    void requestCreateRemoteDir(const QString &path);

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

    // Helpers for keeping logic clean
    void updateConnectionState(bool connected);
    void setRemoteActionsEnabled(bool enabled);
    QString joinPath(const QString &base, const QString &name) const;

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
    QAction *renameLocalAction;
    QAction *deleteRemoteAction;
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
