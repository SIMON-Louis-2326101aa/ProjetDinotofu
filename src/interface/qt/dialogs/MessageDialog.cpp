#include "MessageDialog.hpp"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

MessageDialog::MessageDialog(
    QWidget* parent,
    const QString& title,
    const QString& message
) : QDialog(parent)
{
    setWindowTitle(title);
    setModal(true);
    setMinimumWidth(380);
    setMaximumWidth(460);
    setObjectName("messageDialog");

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 20, 24, 20);
    mainLayout->setSpacing(16);

    auto* titleLabel = new QLabel(title, this);
    titleLabel->setObjectName("dialogTitle");
    titleLabel->setWordWrap(true);

    auto* messageLabel = new QLabel(message, this);
    messageLabel->setObjectName("dialogMessage");
    messageLabel->setWordWrap(true);
    messageLabel->setMinimumHeight(40);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(messageLabel);
}

void MessageDialog::showMessage(
    QWidget* parent,
    const QString& title,
    const QString& message
)
{
    MessageDialog dialog(parent, title, message);

    auto* button = new QPushButton("Compris", &dialog);
    button->setObjectName("dialogConfirmButton");
    button->setMinimumHeight(36);

    dialog.mainLayout->addWidget(button);

    QObject::connect(
        button, &QPushButton::clicked,
        &dialog, &QDialog::accept
    );

    dialog.adjustSize();
    dialog.exec();
}

bool MessageDialog::askConfirmation(
    QWidget* parent,
    const QString& title,
    const QString& message,
    const QString& confirmText,
    const QString& cancelText
)
{
    MessageDialog dialog(parent, title, message);

    auto* buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(12);

    auto* cancelButton = new QPushButton(cancelText, &dialog);
    auto* confirmButton = new QPushButton(confirmText, &dialog);

    cancelButton->setObjectName("dialogCancelButton");
    confirmButton->setObjectName("dialogConfirmButton");

    cancelButton->setMinimumHeight(36);
    confirmButton->setMinimumHeight(36);

    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addWidget(confirmButton);

    dialog.mainLayout->addLayout(buttonsLayout);

    QObject::connect(
        cancelButton, &QPushButton::clicked,
        &dialog, &QDialog::reject
    );

    QObject::connect(
        confirmButton, &QPushButton::clicked,
        &dialog, &QDialog::accept
    );

    dialog.adjustSize();

    return dialog.exec() == QDialog::Accepted;
}
