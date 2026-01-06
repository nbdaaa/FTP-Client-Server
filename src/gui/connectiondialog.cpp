#include "connectiondialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>

ConnectionDialog::ConnectionDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Connect to FTP Server");
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QFormLayout *formLayout = new QFormLayout();

    hostEdit = new QLineEdit(this);
    hostEdit->setText("127.0.0.1");
    hostEdit->setPlaceholderText("e.g., 192.168.1.1");
    formLayout->addRow("Host:", hostEdit);

    portSpinBox = new QSpinBox(this);
    portSpinBox->setRange(1, 65535);
    portSpinBox->setValue(21);
    formLayout->addRow("Port:", portSpinBox);

    usernameEdit = new QLineEdit(this);
    usernameEdit->setText("admin");
    usernameEdit->setPlaceholderText("Username");
    formLayout->addRow("Username:", usernameEdit);

    passwordEdit = new QLineEdit(this);
    passwordEdit->setText("admin123");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("Password");
    formLayout->addRow("Password:", passwordEdit);

    mainLayout->addLayout(formLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    connectButton = new QPushButton("Connect", this);
    connectButton->setDefault(true);
    cancelButton = new QPushButton("Cancel", this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(connectButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    connect(connectButton, &QPushButton::clicked, this, &ConnectionDialog::onConnectClicked);
    connect(cancelButton, &QPushButton::clicked, this, &ConnectionDialog::onCancelClicked);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &ConnectionDialog::onConnectClicked);

    resize(400, 200);
}

QString ConnectionDialog::getHost() const
{
    return hostEdit->text();
}

int ConnectionDialog::getPort() const
{
    return portSpinBox->value();
}

QString ConnectionDialog::getUsername() const
{
    return usernameEdit->text();
}

QString ConnectionDialog::getPassword() const
{
    return passwordEdit->text();
}

void ConnectionDialog::onConnectClicked()
{
    if (hostEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter a host address");
        hostEdit->setFocus();
        return;
    }

    if (usernameEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter a username");
        usernameEdit->setFocus();
        return;
    }

    accept();
}

void ConnectionDialog::onCancelClicked()
{
    reject();
}
