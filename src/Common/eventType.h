//
// Created by monktan on 2021/5/25.
//

#ifndef COMMON_EVENTTYPE_H
#define COMMON_EVENTTYPE_H

namespace mediakit {

typedef enum  {
    None = -1,

    StreamDropped_Normal = 100,       // 正常断流
    StreamDropped_RecvTimeout = 101,  // 接收rtp数据超时
    StreamDropped_Anomaly = 102,      // 流异常

}EventType;

} // namespace mediakit

#endif /* COMMON_EVENTTYPE_H */

