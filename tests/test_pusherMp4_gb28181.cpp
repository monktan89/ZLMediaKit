//
// Created by monktan on 2021/3/29.
//

#include <signal.h>
#include "Util/logger.h"
#include "Util/NoticeCenter.h"
#include "Poller/EventPoller.h"
#include "Player/PlayerProxy.h"
#include "Common/config.h"
#include "Pusher/MediaPusher.h"
#include "Record/MP4Reader.h"

using namespace std;
using namespace toolkit;
using namespace mediakit;

//推流器，保持强引用
MediaPusher::Ptr pusher;
Timer::Ptr g_timer;

struct RTPRemoteInfo {
    bool is_udp;
    uint32_t ssrc;
    uint16_t dst_port;
    uint16_t src_port;
    std::string dst_url;

    RTPRemoteInfo() {
        is_udp = false;
        ssrc = 0;
        dst_port = 0;
        src_port = 0;
    }
};

//声明函数
//推流失败或断开延迟2秒后重试推流
void rePushDelay(const EventPoller::Ptr &poller,
                 const string &schema,
                 const string &vhost,
                 const string &app,
                 const string &stream,
                 const string &filePath,
                 const RTPRemoteInfo &info) ;

//创建推流器并开始推流
void createPusher(const EventPoller::Ptr &poller,
                  const string &schema,
                  const string &vhost,
                  const string &app,
                  const string &stream,
                  const string &filePath,
                  const RTPRemoteInfo &info) {
    //不限制APP名，并且指定文件绝对路径
    auto src = MediaSource::createFromMP4(schema, vhost, app, stream, filePath, false);
    if(!src){
        //文件不存在
        WarnL << "MP4文件不存在:" << filePath;
        return;
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));

    WarnL << "开始发送rtp";

    src->startSendRtp(info.dst_url, info.dst_port, std::to_string(info.ssrc), info.is_udp, info.src_port, [poller,schema,vhost,app,stream,filePath, info](uint16_t local_port, const SockException &ex){
        if (ex) {
            WarnL << "Publish rtp fail:" << ex.getErrCode() << " " << ex.what();
            //如果发布失败，就重试
            rePushDelay(poller,schema,vhost,app, stream, filePath, info);
        }
        InfoL << "push rtp success.";
    });
}

//推流失败或断开延迟2秒后重试推流
void rePushDelay(const EventPoller::Ptr &poller,
                 const string &schema,
                 const string &vhost,
                 const string &app,
                 const string &stream,
                 const string &filePath,
                 const RTPRemoteInfo &info) {
    g_timer = std::make_shared<Timer>(2,[poller,schema,vhost,app, stream, filePath, info]() {
        InfoL << "Re-Publishing...";
        //重新推流
        createPusher(poller,schema,vhost,app, stream, filePath, info);
        //此任务不重复
        return false;
    }, poller);
}

//这里才是真正执行main函数，你可以把函数名(domain)改成main，然后就可以输入自定义url了
int domain(const string & filePath, const RTPRemoteInfo& info){

    auto poller = EventPollerPool::Instance().getPoller();
    //vhost/app/stream可以随便自己填，现在不限制app应用名了
    createPusher(poller, RTMP_SCHEMA, DEFAULT_VHOST, "live", "stream", filePath, info);

    //设置退出信号处理函数
    static semaphore sem;
    signal(SIGINT, [](int) { sem.post(); });// 设置退出信号
    sem.wait();
    pusher.reset();
    g_timer.reset();
    return 0;
}


int main(int argc,char *argv[])
{
    //设置日志
    Logger::Instance().add(std::make_shared<ConsoleChannel>());
    Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

    if(argc < 4){
        ErrorL << "input parameter error, please ./test_pusherMp4_gb28181 src_mp4_file dst_ip dst_port";
        return -1;
    }

    /*
     * argv[1]: 源mp4文件
     * argv[2]: 目标推流地址
     * argv[3]：目标推流端口
     */
    RTPRemoteInfo info;
    info.is_udp = false;
    info.ssrc = 5432678;
    info.dst_port = strtol(argv[3], nullptr, 10);
    info.dst_url = argv[2];
    info.src_port = 30000;  //默认本地30000端口作为源端口推流

    //可以使用test_server生成的mp4文件
    //文件使用绝对路径，推流url支持rtsp和rtmp
    std::string filePath = argv[1];
    return domain(filePath, info);
}
