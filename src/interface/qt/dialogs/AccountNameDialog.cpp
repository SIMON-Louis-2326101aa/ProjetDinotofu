#include "AccountNameDialog.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AccountNameDialog::AccountNameDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("NOUVEAU COMPTE");
    setModal(true);
    setFixedSize(450, 260);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 30, 40, 30);
    layout->setSpacing(15);

    auto* title = new QLabel("NOUVEAU COMPTE", this);
    title->setObjectName("dialogTitle");
    title->setAlignment(Qt::AlignCenter);

    auto* description = new QLabel(
        "Choisis le nom de ton compte local.",
        this
    );
    description->setObjectName("dialogDescription");
    description->setAlignment(Qt::AlignCenter);

    nameInput = new QLineEdit(this);
    nameInput->setPlaceholderText("Nom du compte...");
    nameInput->setObjectName("accountNameInput");

    auto* buttonsLayout = new QHBoxLayout();

    auto* cancelButton = new QPushButton("Annuler", this);
    auto* confirmButton = new QPushButton("Créer", this);

    cancelButton->setObjectName("dialogCancelButton");
    confirmButton->setObjectName("dialogConfirmButton");

    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addWidget(confirmButton);

    layout->addWidget(title);
    layout->addWidget(description);
    layout->addSpacing(10);
    layout->addWidget(nameInput);
    layout->addStretch();
    layout->addLayout(buttonsLayout);

    connect(cancelButton, &QPushButton::clicked,
            this, &QDialog::reject);

    connect(confirmButton, &QPushButton::clicked,
            this, &QDialog::accept);

    nameInput->setFocus();
}

QString AccountNameDialog::accountName() const
{
    return nameInput->text();
}