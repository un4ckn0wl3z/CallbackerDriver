#include "Callbacker.h"

Callbacker::Callbacker(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CallbackerClass())
{
    ui->setupUi(this);
}

Callbacker::~Callbacker()
{
    delete ui;
}
