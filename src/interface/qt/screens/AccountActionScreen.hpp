#ifndef ACCOUNTACTIONSCREEN_HPP
#define ACCOUNTACTIONSCREEN_HPP

#include <QWidget>

#include "save/SaveManager.hpp"

class AccountActionScreen : public QWidget
{
    Q_OBJECT

public:
    explicit AccountActionScreen(
        const AccountSaveSummary& account,
        QWidget* parent = nullptr
    );

signals:
    void backRequested();

private:
    AccountSaveSummary account;
};

#endif