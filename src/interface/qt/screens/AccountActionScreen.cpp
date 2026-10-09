#include "AccountActionScreen.hpp"
#include "../dialogs/MessageDialog.hpp"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QMessageBox>
#include <QString>

AccountActionScreen::AccountActionScreen(
    const AccountSaveSummary& account,
    QWidget* parent
)
    : QWidget(parent),
      account(account)
{
    auto* layout = new QVBoxLayout(this);

    auto* title = new QLabel("COMPTE SÉLECTIONNÉ", this);
    title->setObjectName("gameTitle");
    title->setAlignment(Qt::AlignCenter);

    auto* accountLabel = new QLabel(
        QString("Compte : %1")
            .arg(QString::fromStdString(account.accountName)),
        this
    );

    accountLabel->setAlignment(Qt::AlignCenter);

    auto* charactersLabel = new QLabel(
        QString("Personnages jouables : %1").arg(account.playableCharacterCount),
        this
    );
    charactersLabel->setObjectName("accountStat");
    charactersLabel->setAlignment(Qt::AlignCenter);


    auto* versionLabel = new QLabel(
        QString("Version de sauvegarde : %1")
            .arg(QString::fromStdString(account.savedForVersion)),
        this
    );
    versionLabel->setObjectName("accountStat");
    versionLabel->setAlignment(Qt::AlignCenter);


    auto* activityLabel = new QLabel(
        QString("Dernière activité : %1")
            .arg(QString::fromStdString(account.lastActivityText)),
        this
    );
    activityLabel->setObjectName("accountStat");
    activityLabel->setAlignment(Qt::AlignCenter);

    auto* loginButton = new QPushButton(
        "Se connecter",
        this
    );

    auto* exportButton = new QPushButton(
        "Extraire / transférer ce compte",
        this
    );

    auto* deleteButton = new QPushButton(
        "Supprimer ce compte",
        this
    );

    auto* backButton = new QPushButton(
        "Retour",
        this
    );

    layout->addStretch();

    layout->addWidget(title);
    layout->addSpacing(20);

    layout->addWidget(accountLabel);
    layout->addWidget(charactersLabel);
    layout->addWidget(versionLabel);
    layout->addWidget(activityLabel);

    layout->addSpacing(30);

    layout->addWidget(loginButton, 0, Qt::AlignHCenter);
    layout->addWidget(exportButton, 0, Qt::AlignHCenter);
    layout->addWidget(deleteButton, 0, Qt::AlignHCenter);

    layout->addSpacing(15);

    layout->addWidget(backButton, 0, Qt::AlignHCenter);

    layout->addStretch();


    connect(loginButton, &QPushButton::clicked, this, [this]()
    {
        if (!SaveManager::saveAccountSnapshot(this->account.accountName))
        {
            QMessageBox::warning(
                this,
                "Erreur",
                "Impossible de sélectionner ce compte."
            );
            return;
        }

        QMessageBox::information(
            this,
            "Compte sélectionné",
            QString("Le compte \"%1\" est maintenant actif.")
                .arg(QString::fromStdString(this->account.accountName))
        );
    });

    

    connect(deleteButton, &QPushButton::clicked, this, [this]()
    {
        const bool confirmed = MessageDialog::askConfirmation(
            this,
            "Supprimer le compte",
            QString(
                "Voulez-vous vraiment supprimer le compte \"%1\" ?\n\n"
                "Les personnages et les données associés seront également "
                "supprimés. Cette action est irréversible."
            ).arg(QString::fromStdString(this->account.accountName)),
            "Supprimer",
            "Annuler"
        );

        if (!confirmed)
        {
            return;
        }

        if (SaveManager::deleteAccountAndLinkedCharacters(
                this->account.accountName))
        {
            MessageDialog::showMessage(
                this,
                "Compte supprimé",
                "Le compte et ses données associées ont été supprimés."
            );

            emit backRequested();
        }
        else
        {
            MessageDialog::showMessage(
                this,
                "Erreur",
                "Impossible de supprimer complètement le compte "
                "et ses données associées."
            );
        }
    });

    connect(backButton, &QPushButton::clicked, this, [this]()
    {
        emit backRequested();
    });
}