extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
}

#include <iostream>
#include <vector>

int testFFmpeg(int argc, char* argv[]) {
    // if (argc < 2) {
    //     std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
    //     return -1;
    // }

    const char* filename = "D:/MyVSCode/Testffmpeg/out/1.mp4";
    AVFormatContext* formatContext = nullptr;
    int videoStreamIndex = -1;
    AVCodecContext* codecContext = nullptr;
    AVCodecParameters* codecParams = nullptr;
    // AVCodec* codec = nullptr;
    AVFrame* frame = av_frame_alloc();
    AVPacket packet;
    int gotFrame = 0;
    int ret;
    FILE* outFile = nullptr;

    // 初始化 FFmpeg 库
    // av_register_all();

    std::cout << "test ffmpeg coniler" << std::endl;

    // 打开视频文件
    int ret1 = avformat_open_input(&formatContext, filename, nullptr, nullptr);
    if ( ret1 < 0) {
        std::cerr << "Could not open input file. " <<  std::endl;
        return -1;
    }

    // 读取视频流信息
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        std::cerr << "Could not find stream information." << std::endl;
        return -1;
    }
    // 查找视频流
    for (unsigned i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        std::cerr << "Could not find video stream." << std::endl;
        return -1;
    }

    // 获取视频流的 codec context
    codecParams = formatContext->streams[videoStreamIndex]->codecpar;
    codecContext = avcodec_alloc_context3(nullptr);
    if (avcodec_parameters_to_context(codecContext, codecParams) < 0) {
        std::cerr << "Could not copy codec context." << std::endl;
        return -1;
    }

    // 查找解码器
    const AVCodec* codec = avcodec_find_decoder(codecContext->codec_id);
    if (!codec) {
        std::cerr << "Could not find decoder." << std::endl;
        return -1;
    }

    // 打开解码器
    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        std::cerr << "Could not open codec." << std::endl;
        return -1;
    }

    // 读取第一帧
    while (av_read_frame(formatContext, &packet) >= 0) {
        if (packet.stream_index == videoStreamIndex) {
            ret = avcodec_send_packet(codecContext, &packet);
            if (ret < 0) {
                std::cerr << "Error sending packet to decoder." << std::endl;
                return -1;
            }

            while (ret >= 0) {
                ret = avcodec_receive_frame(codecContext, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    std::cerr << "Error during decoding." << std::endl;
                    return -1;
                }

                //// 保存第一帧
                //if (!gotFrame) {
                //    // 打开输出文件
                //    outFile = fopen("output_frame.png", "wb");
                //    if (!outFile) {
                //        std::cerr << "Could not open output file." << std::endl;
                //        return -1;
                //    }

                //    // 转换像素格式
                //    struct SwsContext* swsContext = sws_getContext(
                //        codecContext->width,
                //        codecContext->height,
                //        codecContext->pix_fmt,
                //        codecContext->width,
                //        codecContext->height,
                //        AV_PIX_FMT_RGB24,
                //        SWS_BILINEAR,
                //        nullptr, nullptr, nullptr
                //    );

                //    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codecContext->width, codecContext->height, 1);
                //    std::vector<uint8_t> buffer(numBytes);
                //    uint8_t* srcSlice[1] = { frame->data[0] };
                //    int srcStride[1] = { frame->linesize[0] };

                //    // 转换帧
                //    sws_scale(swsContext, srcSlice, srcStride, 0, codecContext->height, (uint8_t* const*)(&buffer.data()), nullptr);

                //    // 编码为 PNG
                //    const AVCodec* pngCodec = avcodec_find_encoder(AV_CODEC_ID_PNG);
                //    if (!pngCodec) {
                //        std::cerr << "Could not find PNG encoder." << std::endl;
                //        return -1;
                //    }

                //    AVCodecContext* pngCodecContext = avcodec_alloc_context3(pngCodec);
                //    avcodec_parameters_from_context(avcodec_parameters_alloc(), pngCodecContext);
                //    avcodec_open2(pngCodecContext, pngCodec, nullptr);

                //    // 编码帧
                //    AVFrame* pngFrame = av_frame_alloc();
                //    av_image_alloc();
                //    avpicture_alloc((AVPicture*)pngFrame, AV_PIX_FMT_RGB24, codecContext->width, codecContext->height);
                //    memcpy(pngFrame->data[0], buffer.data(), numBytes);

                //    // 计算帧大小
                //    int pngFrameSize = 0;
                //    uint8_t* pngFrameData[1] = { nullptr };
                //    avcodec_fill_audio_frame(pngFrame, 1, AV_SAMPLE_FMT_U8, (uint8_t*)buffer.data(), numBytes, 0);
                //    av_init_packet(&packet);
                //    packet.data = nullptr;
                //    packet.size = 0;
                //    pngFrameSize = avcodec_encode_video2(pngCodecContext, &packet, pngFrame, &gotFrame);

                //    // 写入文件
                //    fwrite(packet.data, 1, pngFrameSize, outFile);

                //    // 清理
                //    av_free(pngFrame);
                //    avcodec_free_context(&pngCodecContext);
                //    fclose(outFile);
                //    gotFrame = 1;
                //}
            }

            av_packet_unref(&packet);
        }
    }

    // 清理
    avcodec_free_context(&codecContext);
    av_frame_free(&frame);
    avformat_close_input(&formatContext);

    return 0;
}