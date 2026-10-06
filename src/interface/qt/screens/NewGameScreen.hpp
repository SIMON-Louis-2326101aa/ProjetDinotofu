#ifndef NEWGAMESCREEN_HPP
#define NEWGAMESCREEN_HPP

#include <QWidget>

class NewGameScreen : public QWidget
{
    Q_OBJECT

public:
    explicit NewGameScreen(QWidget* parent = nullptr);

signals:
    void backRequested();
};

#endif