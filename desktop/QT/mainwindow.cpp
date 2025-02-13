#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSlider>
#include <QProgressBar>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->verticalSlider, &QSlider:: valueChanged, this, &MainWindow::on_verticalSlider_sliderMoved); // Connect to Slider Bar

    connect(ui->progressBar, &QProgressBar::valueChanged, this, &MainWindow::on_progressBar_valueChanged); // Connect progress bar to slider
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_verticalSlider_sliderMoved(int value)
{
    ui->progressBar->setValue(value);
}

void MainWindow::on_progressBar_valueChanged(int value) // Update slider bar if progress bar changes
{
    ui->verticalSlider->setValue(value);
}


