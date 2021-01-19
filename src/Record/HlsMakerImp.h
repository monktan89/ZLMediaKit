﻿/*
 * Copyright (c) 2016 The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/xia-chu/ZLMediaKit).
 *
 * Use of this source code is governed by MIT license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef HLSMAKERIMP_H
#define HLSMAKERIMP_H

#include <memory>
#include <string>
#include <stdlib.h>
#include "HlsMaker.h"
#include "HlsMediaSource.h"

using namespace std;

namespace mediakit {

class HlsInfo {
public:
    string strFilePath;//m3u8文件路径
    string strAppName;//应用名称
    string strStreamId;//流ID
    time_t ui64StartedTime; //GMT标准时间，单位秒
    time_t ui64TimeLen;//录像长度，单位秒
};

class HlsMakerImp : public HlsMaker{
public:
    HlsMakerImp(const string &m3u8_file,
                const string &params,
                uint32_t bufSize  = 64 * 1024,
                float seg_duration = 5,
                uint32_t seg_number = 3,
                uint32_t record_type = 0);
    ~HlsMakerImp() override;

    /**
     * 设置媒体信息
     * @param vhost 虚拟主机
     * @param app 应用名
     * @param stream_id 流id
     */
    void setMediaSource(const string &vhost, const string &app, const string &stream_id);

    /**
     * 获取MediaSource
     * @return
     */
    HlsMediaSource::Ptr getMediaSource() const;

     /**
      * 清空缓存
      */
     void clearCache();

protected:
    string onOpenSegment(uint64_t index) override ;
    void onDelSegment(uint64_t index) override;
    void onWriteSegment(const char *data, size_t len) override;
    void onWriteHls(const char *data, size_t len) override;
    void onWriteRecordM3u8(const char *header, size_t hlen, const char *body, size_t blen) override;
    void onFlushLastSegment(uint32_t duration_ms) override;

private:
    std::shared_ptr<FILE> makeFile(const string &file,bool setbuf = false);
    std::shared_ptr<FILE> makeRecordM3u8(const string &file,const string &mode,bool setbuf = false);

private:
    int _buf_size;
    string _params;
    string _path_hls;
    string _path_prefix;
    RecordInfo _info;
    std::shared_ptr<FILE> _file;
    std::shared_ptr<char> _file_buf;
    HlsMediaSource::Ptr _media_src;
    map<int /*index*/,string/*file_path*/> _segment_file_paths;
    time_t _ui64StartedTime;
};

}//namespace mediakit
#endif //HLSMAKERIMP_H
