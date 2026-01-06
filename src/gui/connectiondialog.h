#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>

class ConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionDialog(QWidget *parent = nullptr);

    QString getHost() const;
    int getPort() const;
    QString getUsername() const;
    QString getPassword() const;

private slots:
    void onConnectClicked();
    void onCancelClicked();

private:
    QLineEdit *hostEdit;
    QSpinBox *portSpinBox;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *connectButton;
    QPushButton *cancelButton;
};

#endif // CONNECTIONDIALOG_H
