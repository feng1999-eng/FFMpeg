#include "QtWidgetsApplication2.h"

QtWidgetsApplication2::QtWidgetsApplication2(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    connect(ui.playBtn, &QPushButton::clicked, this, &QtWidgetsApplication2::on_clicked_play);
    connect(ui.stopBtn, &QPushButton::clicked, this, &QtWidgetsApplication2::on_clicked_stop);
    connect(ui.playBtn2, &QPushButton::clicked, this, &QtWidgetsApplication2::on_clicked_play_opengl);
    connect(ui.stopBtn, &QPushButton::clicked, this, &QtWidgetsApplication2::on_clicked_stop_opengl);

}


QtWidgetsApplication2::~QtWidgetsApplication2()
{

}

void QtWidgetsApplication2::on_clicked_stop()
{
    ui.widget->stop();
}

void QtWidgetsApplication2::on_clicked_play_audio()
{
    const char* data264 = "C://Users//user//Downloads//128x128.264";
    const char* datamp4 = "D://Code_QT//QtWidgetsApplication2//QtWidgetsApplication2//MP4test.mp4";
    const char* dijia = "D://dijia.qsv";
    ui.widget->setUrl(QString(datamp4));
    ui.widget->play();

}

void QtWidgetsApplication2::on_clicked_stop_audio()
{
}

void QtWidgetsApplication2::on_clicked_play_opengl()
{
    const char* data264 = "C://Users//user//Downloads//128x128.264";
    const char* datamp4 = "D://Code_QT//QtWidgetsApplication2//QtWidgetsApplication2//MP4test.mp4";
    const char* dijia = "D://dijia.qsv";
    ui.openGLWidget->setUrl(QString(datamp4));
    ui.openGLWidget->startVideo();
}

void QtWidgetsApplication2::on_clicked_stop_opengl()
{
    ui.openGLWidget->stop();
}

void QtWidgetsApplication2::on_clicked_play() {
    const char* data264 = "C://Users//user//Downloads//128x128.264";
    const char* datamp4 = "D://Code_QT//QtWidgetsApplication2//QtWidgetsApplication2//MP4test.mp4";
    const char* dijia = "D://dijia.qsv";
    ui.widget->setUrl(QString(datamp4));
    ui.widget->play();
    auto duration = MediaInfo::instance()->get_duration();
    QString info = QString("duration: %1").arg(duration);
    QMessageBox::information(this, "info", info, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    //qDebug() << "duration:   " << duration;
    const QString total_time = QString(std::to_string(duration).c_str());
    ui.label_1_2->setText(total_time);
}
