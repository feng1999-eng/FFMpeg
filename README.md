# FFMpeg
#需要先配置vs2015 ffmpeg4.4 QT5.9 见https://www.cnblogs.com/hailanben/p/17669747.html
![img](https://github.com/feng1999-eng/FFMpeg/blob/main/img/show.png)
左边使用重写QPaintEvent和emit解码得到的每一帧数据转为QImage，响应调用update()实现
右边使用提升openglWidget,使用opengl渲染进行实现
音视频同步暂时使用时钟的方式进行同步
暂停功能通过继承QThread，使用QMutex和QConditionWait以及atomic_bool实现。
