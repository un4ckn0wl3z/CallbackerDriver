#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Callbacker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CallbackerClass; };
QT_END_NAMESPACE

class Callbacker : public QMainWindow
{
    Q_OBJECT

public:
    Callbacker(QWidget *parent = nullptr);
    ~Callbacker();

private:
    Ui::CallbackerClass *ui;
};
