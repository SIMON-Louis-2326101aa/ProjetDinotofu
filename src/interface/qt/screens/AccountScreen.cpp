#include "AccountScreen.hpp"
#include "../dialogs/AccountNameDialog.hpp"
#include "../screens/AccountActionScreen.hpp"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QInputDialog>
#include <QLayout>
#include <QFileDialog>

#include <QDebug>

#include "save/SaveManager.hpp"

AccountScreen::AccountScreen(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    auto* title = new QLabel("COMPTES LOCAUX", this);
    title->setAlignment(Qt::AlignCenter);

    layout->addStretch();
    layout->addWidget(title);
    layout->addSpacing(30);

    auto* newAccountButton = new QPushButton("Créer / utiliser un nouveau compte", this);
    auto* importButton = new QPushButton("Importer un compte extrait", this);
    accountsLayout = new QVBoxLayout();
    auto* backButton = new QPushButton("Retour", this);

    layout->addSpacing(20);
    layout->addWidget(newAccountButton, 0, Qt::AlignHCenter);
    layout->addWidget(importButton, 0, Qt::AlignHCenter);
    layout->addLayout(accountsLayout);
    layout->addWidget(backButton, 0, Qt::AlignHCenter);

    layout->addStretch();
    loadAccounts();

    connect(newAccountButton, &QPushButton::clicked, this, [this]()
    {
        AccountNameDialog dialog(this);

        if (dialog.exec() != QDialog::Accepted)
        {
            return;
        }

        std::string name = dialog.accountName().toStdString();

        if (name.empty())
        {
            name = "local";
        }

        if (!SaveManager::saveAccountSnapshot(name))
        {
            QMessageBox::warning(
                this,
                "Erreur",
                "Impossible de créer le compte."
            );
            return;
        }

        loadAccounts();
    });

    connect(importButton, &QPushButton::clicked, this, [this]()
    {
        QString packagePath = QFileDialog::getExistingDirectory(
            this,
            "Sélectionner le compte extrait",
            "assets/saves",
            QFileDialog::ShowDirsOnly
        );

        if (packagePath.isEmpty())
        {
            return;
        }

        std::string importedAccountName;

        if (SaveManager::importAccountPackage(
                packagePath.toStdString(),
                importedAccountName
            ))
        {
            QMessageBox::information(
                this,
                "Compte importé",
                QString("Le compte \"%1\" a été importé avec succès.")
                    .arg(QString::fromStdString(importedAccountName))
            );

            loadAccounts();
        }
        else
        {
            QMessageBox::warning(
                this,
                "Import impossible",
                "Impossible d'importer ce compte.\n\n"
                "Vérifie que tu as sélectionné un dossier de compte "
                "correctement extrait."
            );
        }
    });

    connect(backButton, &QPushButton::clicked, this, [this]()
    {
        emit backRequested();
    });
}

void AccountScreen::loadAccounts()
{
    std::vector<AccountSaveSummary> accounts = SaveManager::listAccounts();

    while (QLayoutItem* item = accountsLayout->takeAt(0))
    {
        if (QWidget* widget = item->widget())
        {
            widget->deleteLater();
        }

        delete item;
    }

    for (const AccountSaveSummary& account : accounts)
    {
        auto* button = new QPushButton(
            QString::fromStdString(account.accountName),
            this
        );

        button->setToolTip(
            QString("Personnages : %1\nVersion : %2\nDernière activité : %3")
                .arg(account.playableCharacterCount)
                .arg(QString::fromStdString(account.savedForVersion))
                .arg(QString::fromStdString(account.lastActivityText))
        );

        accountsLayout->addWidget(
            button,
            0,
            Qt::AlignHCenter
        );

        connect(button, &QPushButton::clicked, this, [this, account]()
        {
            auto* actionScreen = new AccountActionScreen(account, this);

            emit accountSelected(actionScreen);
        });
    }
}