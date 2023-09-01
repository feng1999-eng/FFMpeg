#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtWidgetsApplication2.h"
#include "qpushbutton.h"
#include "qlineedit.h"
#include "qfiledialog.h"
#include "FFmpegWidget.h"
#include "qdatetime.h"
#include "MediaInfo.h"
#include "qmessagebox.h"

class QtWidgetsApplication2 : public QMainWindow
{
    Q_OBJECT

public:
    QtWidgetsApplication2(QWidget *parent = nullptr);
    ~QtWidgetsApplication2();
private:
    Ui::QtWidgetsApplication2Class ui;
    
protected slots:
    void on_clicked_play();
    void on_clicked_stop();
    void on_clicked_play_audio();
    void on_clicked_stop_audio();
    void on_clicked_play_opengl();
    void on_clicked_stop_opengl();
};
