﻿/*
 * Copyright (c) 2016 The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/xia-chu/ZLMediaKit).
 *
 * Use of this source code is governed by MIT license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef ZLMEDIAKIT_WEBAPI_H
#define ZLMEDIAKIT_WEBAPI_H

#include <string>
#include <functional>
#include "jsoncpp/json.h"
#include "Common/Parser.h"
#include "Network/Socket.h"
#include "Http/HttpSession.h"

using namespace std;
using namespace Json;
using namespace toolkit;
using namespace mediakit;

//配置文件路径
extern string g_ini_file;

namespace mediakit {
////////////RTSP服务器配置///////////
namespace Rtsp {
extern const string kPort;
} //namespace Rtsp

////////////RTMP服务器配置///////////
namespace Rtmp {
extern const string kPort;
} //namespace RTMP
}  // namespace mediakit

namespace API {
typedef enum {
    NotFound = -500,//未找到
    Exception = -400,//代码抛异常
    InvalidArgs = -300,//参数不合法
    SqlFailed = -200,//sql执行失败
    AuthFailed = -100,//鉴权失败
    OtherFailed = -1,//业务代码执行失败，
    Success = 0//执行成功
} ApiErr;
}//namespace API

class ApiRetException: public std::runtime_error {
public:
    ApiRetException(const char *str = "success" ,int code = API::Success):runtime_error(str){
        _code = code;
    }
    ~ApiRetException() = default;
    int code(){ return _code; }
private:
    int _code;
};

class AuthException : public ApiRetException {
public:
    AuthException(const char *str):ApiRetException(str,API::AuthFailed){}
    ~AuthException() = default;
};

class InvalidArgsException: public ApiRetException {
public:
    InvalidArgsException(const char *str):ApiRetException(str,API::InvalidArgs){}
    ~InvalidArgsException() = default;
};

class SuccessException: public ApiRetException {
public:
    SuccessException():ApiRetException("success",API::Success){}
    ~SuccessException() = default;
};

using ApiArgsType = map<string, variant, StrCaseCompare>;

#define API_ARGS_MAP SockInfo &sender, HttpSession::KeyValue &headerIn, HttpSession::KeyValue &headerOut, ApiArgsType &allArgs, Json::Value &val
#define API_ARGS_MAP_ASYNC API_ARGS_MAP, const HttpSession::HttpResponseInvoker &invoker
#define API_ARGS_JSON SockInfo &sender, HttpSession::KeyValue &headerIn, HttpSession::KeyValue &headerOut, Json::Value &allArgs, Json::Value &val
#define API_ARGS_JSON_ASYNC API_ARGS_JSON, const HttpSession::HttpResponseInvoker &invoker
#define API_ARGS_VALUE sender, headerIn, headerOut, allArgs, val

//注册http请求参数是map<string, variant, StrCaseCompare>类型的http api
void api_regist(const string &api_path, const function<void(API_ARGS_MAP)> &func);
//注册http请求参数是map<string, variant, StrCaseCompare>类型,但是可以异步回复的的http api
void api_regist(const string &api_path, const function<void(API_ARGS_MAP_ASYNC)> &func);

//注册http请求参数是Json::Value类型的http api(可以支持多级嵌套的json参数对象)
void api_regist(const string &api_path, const function<void(API_ARGS_JSON)> &func);
//注册http请求参数是Json::Value类型，但是可以异步回复的的http api
void api_regist(const string &api_path, const function<void(API_ARGS_JSON_ASYNC)> &func);

template<typename Args, typename First>
bool checkArgs(Args &&args, First &&first) {
    return !args[first].empty();
}

template<typename Args, typename First, typename ...KeyTypes>
bool checkArgs(Args &&args, First &&first, KeyTypes &&...keys) {
    return !args[first].empty() && checkArgs(std::forward<Args>(args), std::forward<KeyTypes>(keys)...);
}

//检查http参数是否为空的宏
#define CHECK_ARGS(...)  \
    if(!checkArgs(allArgs,##__VA_ARGS__)){ \
        throw InvalidArgsException("缺少必要参数:" #__VA_ARGS__); \
    }

//检查http参数中是否附带secret密钥的宏，127.0.0.1的ip不检查密钥
#define CHECK_SECRET() \
    if(sender.get_peer_ip() != "127.0.0.1"){ \
        CHECK_ARGS("secret"); \
        if(api_secret != allArgs["secret"]){ \
            throw AuthException("secret错误"); \
        } \
    }

void installWebApi();
void unInstallWebApi();

class ProxyPusherInfo {
public:
    string key; //流id的key
    string proxy_pusher_url;//转推地址
};

#endif //ZLMEDIAKIT_WEBAPI_H
