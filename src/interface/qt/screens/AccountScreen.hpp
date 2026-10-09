#ifndef ACCOUNTSCREEN_HPP
#define ACCOUNTSCREEN_HPP

#include <QWidget>

class QVBoxLayout;


class AccountScreen : public QWidget
{
    Q_OBJECT

public:
    explicit AccountScreen(QWidget* parent = nullptr);

signals:
    void backRequested();
    void accountSelected(QWidget* screen);

    
private:
    void loadAccounts();

    QVBoxLayout* accountsLayout;

};

#endif