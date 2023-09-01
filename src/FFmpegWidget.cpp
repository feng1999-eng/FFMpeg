#include "ffmpegwidget.h"
#include "qdebug.h"
FFmpegAudio::FFmpegAudio()
{
    fmtCtx = avformat_alloc_context();
    pkt = av_packet_alloc();
    audioFrame = av_frame_alloc();
    QAudioFormat audioFmt;
    audioFmt.setSampleRate(44100);
    audioFmt.setChannelCount(2);
    audioFmt.setSampleSize(16);
    audioFmt.setCodec("audio/pcm");
    audioFmt.setByteOrder(QAudioFormat::LittleEndian);
    audioFmt.setSampleType(QAudioFormat::SignedInt);
    QAudioDeviceInfo info = QAudioDeviceInfo::defaultOutputDevice();
    if (!info.isFormatSupported(audioFmt)) {
        audioFmt = info.nearestFormat(audioFmt);
    }
    audioOutput = new QAudioOutput(audioFmt);
    audioOutput->setVolume(100);
    streamOut = audioOutput->start();
}

FFmpegAudio::~FFmpegAudio()
{
    if (streamOut->isOpen()) {
        audioOutput->stop();
        streamOut->close();
    }
    if (!pkt) av_packet_free(&pkt);
    if (!audioCodecCtx) avcodec_free_context(&audioCodecCtx);
    if (!audioCodecCtx) avcodec_close(audioCodecCtx);
    if (!fmtCtx) avformat_close_input(&fmtCtx);
}

void FFmpegAudio::setUrl(QString url)
{
    _url = url;
}

int FFmpegAudio::open_input_file()
{
    if (_url.isEmpty()) return -1;

    if (avformat_open_input(&fmtCtx, _url.toLocal8Bit().data(), NULL, NULL) < 0) {
        printf("Cannot open input file.\n");
        return -1;
    }

    if (avformat_find_stream_info(fmtCtx, NULL) < 0) {
        printf("Cannot find any stream in file.\n");       
        return -1;
    }

    int streamCnt = fmtCtx->nb_streams;
    for (int i = 0; i < streamCnt; i++) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = (int)i;
            continue;
        }
    }

    if (audioStreamIndex == -1) {
        printf("Cannot find any stream in file.\n");
        return -1;
    }

    ///////////////////////////��Ƶ���ֿ�ʼ//////////////////////////////////
    AVCodecParameters* audioCodecPara = fmtCtx->streams[audioStreamIndex]->codecpar;

    if (!(audioCodec = avcodec_find_decoder(audioCodecPara->codec_id))) {
        printf("Cannot find valid audio decode codec.\n");
        return -1;
    }

    if (!(audioCodecCtx = avcodec_alloc_context3(audioCodec))) {
        printf("Cannot find valid audio decode codec context.\n");
        return -1;
    }

    if (avcodec_parameters_to_context(audioCodecCtx, audioCodecPara) < 0) {
        printf("Cannot initialize audio parameters.\n");
        return -1;
    }

    audioCodecCtx->pkt_timebase = fmtCtx->streams[audioStreamIndex]->time_base;

    if (avcodec_open2(audioCodecCtx, audioCodec, NULL) < 0) {
        printf("Cannot open audio codec.\n");
        return -1;
    }

    //����ת�����
    uint64_t out_channel_layout = audioCodecCtx->channel_layout;
    out_sample_rate = audioCodecCtx->sample_rate;
    out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
    //printf("out rate : %d , out_channel is: %d\n",out_sample_rate,out_channels);

    audio_out_buffer = (uint8_t*)av_malloc(MAX_AUDIO_FRAME_SIZE * 2);

    swr_ctx = swr_alloc_set_opts(NULL,
        out_channel_layout,
        out_sample_fmt,
        out_sample_rate,
        audioCodecCtx->channel_layout,
        audioCodecCtx->sample_fmt,
        audioCodecCtx->sample_rate,
        0, NULL);
    swr_init(swr_ctx);
    ///////////////////////////��Ƶ���ֽ���//////////////////////////////////

    return true;
}

void FFmpegAudio::pause()
{
    is_pause_audio = true;
}

void FFmpegAudio::resume()
{
    is_pause_audio = false;
    m_cond_audio.wakeAll();
}

void FFmpegAudio::run()
{
    if (!open_input_file()) {
        qDebug() << "Please open file first.";
        return;
    }

    qDebug() << "Open file " << _url << "done.";

    double sleep_time = 0;

    while (av_read_frame(fmtCtx, pkt) >= 0) {
        m_mutex_audio.lock();
        if (is_pause_audio) {
            m_cond_audio.wait(&m_mutex_audio);
        }
        m_mutex_audio.unlock();
        qDebug() << "read audio frame";
        if (pkt->stream_index == audioStreamIndex) {
            qDebug() << "audio";
            if (avcodec_send_packet(audioCodecCtx, pkt) >= 0) {
                while (avcodec_receive_frame(audioCodecCtx, audioFrame) >= 0) {
                    qDebug() << "receive";
                    if (av_sample_fmt_is_planar(audioCodecCtx->sample_fmt)) {
                        int len = swr_convert(swr_ctx,
                            &audio_out_buffer,
                            MAX_AUDIO_FRAME_SIZE * 2,
                            (const uint8_t**)audioFrame->data,
                            audioFrame->nb_samples);
                        if (len <= 0) {
                            continue;
                        }
                        //qDebug("convert length is: %d.\n",len);

                        int out_size = av_samples_get_buffer_size(0,
                            out_channels,
                            len,
                            out_sample_fmt,
                            1);

                        sleep_time = (out_sample_rate * 16 * 2 / 8) / out_size;

                        if (audioOutput->bytesFree() < out_size) {
                            msleep(sleep_time);
                            streamOut->write((char*)audio_out_buffer, out_size);
                        }
                        else {
                            streamOut->write((char*)audio_out_buffer, out_size);
                        }
                    }
                }
            }
            av_packet_unref(pkt);
        }
    }

    qDebug() << "All video play done";
}



FFmpegVideo::FFmpegVideo()
{
    fmtCtx = avformat_alloc_context();
    pkt = av_packet_alloc();
    yuvFrame = av_frame_alloc();
    rgbFrame = av_frame_alloc();
}

FFmpegVideo::~FFmpegVideo()
{
    if (!pkt) av_packet_free(&pkt);
    if (!yuvFrame) av_frame_free(&yuvFrame);
    if (!rgbFrame) av_frame_free(&rgbFrame);
    if (!videoCodecCtx) avcodec_free_context(&videoCodecCtx);
    if (!videoCodecCtx) avcodec_close(videoCodecCtx);
    if (!fmtCtx) avformat_close_input(&fmtCtx);
}

void FFmpegVideo::setUrl(QString url)
{
    _url = url;
}

int FFmpegVideo::open_input_file()
{
    if (_url.isEmpty()) return -1;

    if (avformat_open_input(&fmtCtx, _url.toLocal8Bit().data(), NULL, NULL) < 0) {  //���ļ�
        printf("Cannot open input file.\n");
        return -1;
    }

    if (avformat_find_stream_info(fmtCtx, NULL) < 0) {
        printf("Cannot find any stream in file.\n");
        return -1;
    }
    
    //QMessageBox::information(nullptr, "info", QString(MediaInfo::instance()->get_duration()), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    int streamCnt = fmtCtx->nb_streams;          //��������Ϣ����Ƶ������
    for (int i = 0; i < streamCnt; i++) {
        if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            continue;
        }
    }

    if (videoStreamIndex == -1) {
        printf("Cannot find video stream in file.\n");
        return -1;
    }

    AVCodecParameters* videoCodecPara = fmtCtx->streams[videoStreamIndex]->codecpar;//��ȡ��Ƶ���ı����������

    if (!(videoCodec = avcodec_find_decoder(videoCodecPara->codec_id))) {//����ָ���ı������id�������Ʋ��Ҷ�Ӧ�ı������
        printf("Cannot find valid decode codec.\n");
        return -1;
    }

    if (!(videoCodecCtx = avcodec_alloc_context3(videoCodec))) {  //����һ��AVCodecContext�ṹ��
        printf("Cannot find valid decode codec context.\n");
        return -1;
    }

    if (avcodec_parameters_to_context(videoCodecCtx, videoCodecPara) < 0) {//��AVCodecParameters�ṹ���еĲ�����䵽AVCodecContext�ṹ����
        printf("Cannot initialize parameters.\n");
        return -1;
    }
    if (avcodec_open2(videoCodecCtx, videoCodec, NULL) < 0) {//���ڴ�AVCodecContext����AVCodec���й���
        printf("Cannot open codec.\n");
        return -1;
    }
    img_ctx = sws_getContext(videoCodecCtx->width,//��������������
        videoCodecCtx->height,
        videoCodecCtx->pix_fmt,
        videoCodecCtx->width,
        videoCodecCtx->height,
        AV_PIX_FMT_RGB32,
        SWS_BICUBIC, NULL, NULL, NULL);

    numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, videoCodecCtx->width, videoCodecCtx->height, 1);//���ڼ������ͼ�������ͼ�����ݻ�������С
    out_buffer = (unsigned char*)av_malloc(numBytes * sizeof(unsigned char));//�����ڴ棬�ͷ���Ҫ����av_free()

    int res = av_image_fill_arrays(
        rgbFrame->data, rgbFrame->linesize,
        out_buffer, AV_PIX_FMT_RGB32,
        videoCodecCtx->width, videoCodecCtx->height, 1);
    if (res < 0) {
        qDebug() << "Fill arrays failed.";
        return -1;
    }
    MediaInfo::instance()->set_duration(fmtCtx->duration / AV_TIME_BASE);
    return true;
}

void FFmpegVideo::run()
{

    if (!open_input_file()) {
        qDebug() << "Please open file first.";
        return;
    }

    while (av_read_frame(fmtCtx, pkt) >= 0) {//��ʼѭ����ȡ��Ƶ֡��AVFormatContext�洢�˴��ļ���ȫ����Ϣ
        m_mutex_video.lock();
        if (is_pause_video) {
            m_cond_video.wait(&m_mutex_video);
        }
        m_mutex_video.unlock();
        if (pkt->stream_index == videoStreamIndex) {
            if (avcodec_send_packet(videoCodecCtx, pkt) >= 0) {//�������ݵ�ffmpeg�ŵ�������У�
                int ret;
                while ((ret = avcodec_receive_frame(videoCodecCtx, yuvFrame)) >= 0) {//���ɹ��Ľ��������ȡ��һ��frame
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                        return;
                    else if (ret < 0) {
                        fprintf(stderr, "Error during decoding\n");
                        exit(1);
                    }
                    sws_scale(img_ctx,//ִ��ͼ������ź���ɫ�ռ��ת��
                        yuvFrame->data, yuvFrame->linesize,
                        0, videoCodecCtx->height,
                        rgbFrame->data, rgbFrame->linesize);

                    QImage img(out_buffer,//out_bufferָ�����ͼ�����ݵ���ʼλ��ָ�룬�ǲ����ĸı�������������
                        videoCodecCtx->width, videoCodecCtx->height,
                        QImage::Format_RGB32);
                    emit sendQImage(img);
                    QThread::msleep(30);
                }
            }
            av_packet_unref(pkt);
        }
    }
    MediaInfo::instance()->set_duration(fmtCtx->duration / AV_TIME_BASE);
    qDebug() << "All video play done";
}

void FFmpegVideo::pause()
{
    is_pause_video = true;
}

void FFmpegVideo::resume()
{
    is_pause_video = false;
    m_cond_video.wakeAll();
}

FFmpegWidget::FFmpegWidget(QWidget* parent) : QWidget(parent)
{
    ffmpeg = new FFmpegVideo;
    connect(ffmpeg, SIGNAL(sendQImage(QImage)), this, SLOT(receiveQImage(QImage)));
    connect(ffmpeg, &FFmpegVideo::finished, ffmpeg, &FFmpegVideo::deleteLater);

    ffaudio = new FFmpegAudio;
    connect(ffaudio, &FFmpegAudio::finished, ffaudio, &FFmpegAudio::deleteLater);
}

FFmpegWidget::~FFmpegWidget()
{
    if (ffmpeg->isRunning()) {
        stop();
    }
}

void FFmpegWidget::setUrl(QString url)
{
    ffmpeg->setUrl(url);
    ffaudio->setUrl(url);
}

void FFmpegWidget::play()
{
    stop();
    ffmpeg->resume();
    ffmpeg->start();
    ffaudio->resume();
    ffaudio->start();
    qDebug() << u8"������";
}

void FFmpegWidget::stop()
{
    qDebug() << u8"����ͣ";
    ffmpeg->pause();
    ffaudio->pause();
}

void FFmpegWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawImage(0, 0, img);
}

void FFmpegWidget::receiveQImage(const QImage& rImg)
{
    
    img = rImg.scaled(this->size());
    update();
}