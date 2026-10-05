#include "MainWindow.hpp"

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QApplication>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Dinotofu");
    resize(1000, 700);

    auto* centralWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(centralWidget);

    auto* title = new QLabel("DINOTOFU", centralWidget);
    title->setAlignment(Qt::AlignCenter);

    auto* newGameButton = new QPushButton("Nouvelle partie", centralWidget);
    auto* continueButton = new QPushButton("Continuer", centralWidget);
    auto* optionsButton = new QPushButton("Options", centralWidget);
    auto* quitButton = new QPushButton("Quitter", centralWidget);

    connect(quitButton, &QPushButton::clicked, this, [this]()
{
    close();
});

    newGameButton->setObjectName("newGameButton");
    continueButton->setObjectName("continueButton");
    optionsButton->setObjectName("optionsButton");
    quitButton->setObjectName("quitButton");

    layout->addStretch();

    layout->addWidget(title);
    layout->addSpacing(30);

    layout->addWidget(newGameButton);
    layout->addWidget(continueButton);
    layout->addWidget(optionsButton);
    layout->addWidget(quitButton);

    layout->addStretch();

    setCentralWidget(centralWidget);
}