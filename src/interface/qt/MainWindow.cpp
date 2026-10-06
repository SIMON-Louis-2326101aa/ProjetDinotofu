#include "MainWindow.hpp"
#include "screens/NewGameScreen.hpp"
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Dinotofu");
    resize(1468, 825);

    showMainMenu();
}

void MainWindow::showMainMenu()
{
    auto* centralWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(centralWidget);

    auto* title = new QLabel("DINOTOFU", centralWidget);
    title->setAlignment(Qt::AlignCenter);
    title->setObjectName("MainTitle");

    auto* newGameButton = new QPushButton("Nouvelle partie", centralWidget);
    auto* continueButton = new QPushButton("Continuer", centralWidget);
    auto* optionsButton = new QPushButton("Options", centralWidget);
    auto* quitButton = new QPushButton("Quitter", centralWidget);

    layout->addStretch();

    layout->addWidget(title);
    layout->addSpacing(30);

    layout->addWidget(newGameButton, 0, Qt::AlignHCenter);
    layout->addWidget(continueButton, 0, Qt::AlignHCenter);
    layout->addWidget(optionsButton, 0, Qt::AlignHCenter);
    layout->addWidget(quitButton, 0, Qt::AlignHCenter);

    layout->addStretch();

    setCentralWidget(centralWidget);

    connect(newGameButton, &QPushButton::clicked, this, [this]()
    {
        showNewGameScreen();
    });

    connect(quitButton, &QPushButton::clicked, this, [this]()
    {
        close();
    });
}

void MainWindow::showNewGameScreen()
{
    auto* newGameScreen = new NewGameScreen(this);

    setCentralWidget(newGameScreen);

    connect(newGameScreen, &NewGameScreen::backRequested,
            this, [this]()
    {
        showMainMenu();
    });
}