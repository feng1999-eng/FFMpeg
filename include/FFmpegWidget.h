#ifndef FFMPEGWIDGET_H
#define FFMPEGWIDGET_H

#include <QImage>
#include <QWidget>
#include <QPaintEvent>
#include <QThread>
#include <QPainter>
#include <QDebug>
#include "qmutex.h"
#include "qwaitcondition.h"
#include <string>
#include "qiodevice.h"
#include "qaudiooutput.h"
#include "MediaInfo.h"
#include "qmessagebox.h"

extern "C" {
#include <libavcodec/avcodec.h>

#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/parseutils.h>
#include <libavutil/mem.h>

#include <libswscale/swscale.h>

#include <libavformat/avformat.h>

#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
}

using namespace std;


#define MAX_AUDIO_FRAME_SIZE 192000

class FFmpegAudio :public QThread {
    Q_OBJECT
public:
    explicit FFmpegAudio();
    ~FFmpegAudio();
    void setUrl(QString url);
    int open_input_file();
    void pause();
    void resume();
protected:
    void run();
private:
    AVFormatContext* fmtCtx = nullptr;
    const AVCodec* audioCodec = nullptr;
    AVCodecContext* audioCodecCtx = nullptr;
    AVPacket* pkt = nullptr;
    AVFrame* audioFrame = nullptr;
    struct SwrContext* swr_ctx = nullptr;
    uint8_t* audio_out_buffer = nullptr;
    int out_channels;
    int out_sample_rate;
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    int audioStreamIndex = -1;
    int numBytes = -1;
    QAudioOutput* audioOutput;
    QIODevice* streamOut;
    QString _url;

    QMutex m_mutex_audio;
    QWaitCondition m_cond_audio;
    std::atomic_bool is_pause_audio;
};

class FFmpegVideo : public QThread
{
    Q_OBJECT
public:
    explicit FFmpegVideo();
    ~FFmpegVideo();

    void setUrl(QString url);

    int open_input_file();
    void pause();
    void resume();
protected:
    void run();

signals:
    void sendQImage(QImage);

private:
    AVFormatContext* fmtCtx = NULL;
    const AVCodec* videoCodec = NULL;
    AVCodecContext* videoCodecCtx = NULL;
    AVPacket* pkt = NULL;
    AVFrame* yuvFrame = NULL;
    AVFrame* rgbFrame = NULL;

    struct SwsContext* img_ctx = NULL;

    unsigned char* out_buffer = nullptr;

    int videoStreamIndex = -1;
    int numBytes = -1;
    QString _url;

    QMutex m_mutex_video;
    QWaitCondition m_cond_video;
    std::atomic_bool is_pause_video;
    int64_t duration;
};

class FFmpegWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FFmpegWidget(QWidget* parent = nullptr);
    ~FFmpegWidget();

    void setUrl(QString url);

    void play();
    void stop();

protected:
    void paintEvent(QPaintEvent*);

private slots:
    void receiveQImage(const QImage& rImg);

private:
    FFmpegVideo* ffmpeg;
    FFmpegAudio* ffaudio;
    QImage img;
};

#endif // FFMPEGWIDGET_H