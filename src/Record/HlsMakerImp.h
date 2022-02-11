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

namespace mediakit {

class HlsInfo {
public:
    std::string strFilePath;	//m3u8文件路径
    std::string strAppName;		//应用名称
    std::string strStreamId;	//流ID
    time_t ui64StartedTime; 	//GMT标准时间，单位秒
    time_t ui64TimeLen;			//录像长度，单位秒
};

class HlsMakerImp : public HlsMaker{
public:
    HlsMakerImp(const std::string &m3u8_file,
                const std::string &params,
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
    void setMediaSource(const std::string &vhost, const std::string &app, const std::string &stream_id);

    /**
     * 获取MediaSource
     * @return
     */
    HlsMediaSource::Ptr getMediaSource() const;

     /**
      * 清空缓存
      * @param immediately 时候立即删除
      */
     void clearCache(bool isFirst = false, bool immediately = true);

protected:
    std::string onOpenSegment(uint64_t index) override ;
    void onDelSegment(uint64_t index) override;
    void onWriteSegment(const char *data, size_t len) override;
    void onWriteHls(const std::string &data) override;
    void onWriteRecordM3u8(const char *header, size_t hlen, const char *body, size_t blen) override;
    void onFlushLastSegment(uint32_t duration_ms) override;

private:
    std::shared_ptr<FILE> makeFile(const std::string &file, bool setbuf = false);
    std::shared_ptr<FILE> makeRecordM3u8(const std::string &file, const std::string &mode, bool setbuf = false);

private:
    int _buf_size;
    std::string _params;
    std::string _path_hls;
    std::string _path_prefix;
    RecordInfo _info;
    std::shared_ptr<FILE> _file;
    std::shared_ptr<char> _file_buf;
    HlsMediaSource::Ptr _media_src;
    toolkit::EventPoller::Ptr _poller;
    std::map<uint64_t /*index*/,std::string/*file_path*/> _segment_file_paths;
    time_t _ui64StartedTime;
    uint8_t _hls_type;
};

}//namespace mediakit
#endif //HLSMAKERIMP_H
