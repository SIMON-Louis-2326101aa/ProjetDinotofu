#ifndef MESSAGEDIALOG_HPP
#define MESSAGEDIALOG_HPP

#include <QDialog>
#include <QString>

class QVBoxLayout;
class QWidget;

class MessageDialog : public QDialog
{
public:
    explicit MessageDialog(
        QWidget* parent,
        const QString& title,
        const QString& message
    );

    static void showMessage(
        QWidget* parent,
        const QString& title,
        const QString& message
    );

    static bool askConfirmation(
        QWidget* parent,
        const QString& title,
        const QString& message,
        const QString& confirmText = "Confirmer",
        const QString& cancelText = "Annuler"
    );

private:
    QVBoxLayout* mainLayout;
};

#endif