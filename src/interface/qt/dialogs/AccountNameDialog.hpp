#ifndef ACCOUNTNAMEDIALOG_HPP
#define ACCOUNTNAMEDIALOG_HPP

#include <QDialog>

class QLineEdit;

class AccountNameDialog : public QDialog
{
public:
    explicit AccountNameDialog(QWidget* parent = nullptr);

    QString accountName() const;

private:
    QLineEdit* nameInput;
};

#endif