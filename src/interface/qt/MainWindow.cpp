#include "MainWindow.hpp"
#include "screens/AccountScreen.hpp"
#include "screens/AccountActionScreen.hpp"

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

    auto* account = new QPushButton("Se connecter / Crée un compte", centralWidget);
    auto* optionsButton = new QPushButton("Options", centralWidget);
    auto* quitButton = new QPushButton("Quitter", centralWidget);

    layout->addStretch();

    layout->addWidget(title);
    layout->addSpacing(30);

    layout->addWidget(account, 0, Qt::AlignHCenter);
    layout->addWidget(optionsButton, 0, Qt::AlignHCenter);
    layout->addWidget(quitButton, 0, Qt::AlignHCenter);

    layout->addStretch();

    setCentralWidget(centralWidget);

    connect(account, &QPushButton::clicked, this, [this]()
    {
        showAccountScreen();
    });

    connect(quitButton, &QPushButton::clicked, this, [this]()
    {
        close();
    });
}

void MainWindow::showAccountScreen()
{
    auto* accountScreen = new AccountScreen(this);

    setCentralWidget(accountScreen);

    connect(accountScreen, &AccountScreen::backRequested,
            this, [this]()
            {
                showMainMenu();
            });

    connect(accountScreen, &AccountScreen::accountSelected,
            this, [this](QWidget* screen)
            {
                setCentralWidget(screen);

                auto* actionScreen =
                    qobject_cast<AccountActionScreen*>(screen);

                if (actionScreen == nullptr)
                {
                    return;
                }

                connect(actionScreen, &AccountActionScreen::backRequested,
                        this, [this]()
                        {
                            showAccountScreen();
                        });
            });
}