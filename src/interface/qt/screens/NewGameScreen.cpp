#include "NewGameScreen.hpp"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

NewGameScreen::NewGameScreen(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    auto* title = new QLabel("NOUVELLE PARTIE", this);
    title->setAlignment(Qt::AlignCenter);

    auto* startButton = new QPushButton("Commencer", this);
    auto* backButton = new QPushButton("Retour", this);

    layout->addStretch();
    layout->addWidget(title);
    layout->addSpacing(30);
    layout->addWidget(startButton, 0, Qt::AlignHCenter);
    layout->addWidget(backButton, 0, Qt::AlignHCenter);
    layout->addStretch();

    connect(backButton, &QPushButton::clicked, this, [this]()
    {
        emit backRequested();
    });
}