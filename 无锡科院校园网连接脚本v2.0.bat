@echo off
:: 设置变量
set "USERNAME=你的学号@运营商" 
set "PASSWORD=你的身份证后六位密码"
::中国移动 cmcc
::中国联通 unicom
::中国电信 telecom
::例 "USERNAME=2025*******@cmcc"  密码身份证后6位
set "LOGIN_URL=http://10.255.254.1:801/eportal/?c=Portal&a=login&callback=dr1003&login_method=1&user_account=,0,%USERNAME%&user_password=%PASSWORD%&wlan_user_ip=10.16.41.102&wlan_user_ipv6=&wlan_user_mac=000000000000&wlan_user_ac_ip=&wlan_ac_name=&jsVersion=3.3.2&v=9850"
set "LOGOUT_URL=http://10.255.254.1:801/eportal/?c=Portal&a=logout&callback=dr1004&login_method=1&user_account=drcom&user_password=123&ac_logout=1&register_mode=1&wlan_user_ip=10.16.41.102&wlan_user_ipv6=&wlan_vlan_id=1&wlan_user_mac=345a60b68e84&wlan_ac_ip=&wlan_ac_name=&jsVersion=3.3.2&v=850"
set "UNBIND_MAC_URL=http://10.255.254.1:801/eportal/?c=Portal&a=unbind_mac&callback=dr1003&user_account=%USERNAME%&wlan_user_mac=345A60B68E84&wlan_user_ip=10.16.41.102&jsVersion=3.3.2&v=8416"
set "TEST_URL=www.baidu.com"

:: 创建临时文件存储响应
set "RESPONSE_FILE=%temp%\login_response.txt"

echo [%date% %time%] 
echo 开始校园网登录流程...

echo 正在检测网络状态...
curl -s --head --connect-timeout 3 http://www.baidu.com | findstr /r /c:"HTTP/[0-9]\.[0-9] 200" >nul
if %errorlevel%==0 (
    echo 当前已可以上网，正在执行注销操作...
    curl -s "%LOGOUT_URL%" ^
      -H "User-Agent: Mozilla/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36" ^
      -H "Referer: http://10.255.254.1:801/" ^
      -H "Accept: */*" ^
      -H "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8" > "%RESPONSE_FILE%"
    timeout /t 5 /nobreak >nul
    echo 正在解除MAC绑定...
    curl -s "%UNBIND_MAC_URL%" ^
      -H "User-Agent: Mozilla/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36" ^
      -H "Referer: http://10.255.254.1:801/" ^
      -H "Accept: */*" ^
      -H "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8" > "%RESPONSE_FILE%"
    timeout /t 3 /nobreak >nul
)

:: 使用 curl 发送 GET 请求
curl -s "%LOGIN_URL%" ^
  -H "User-Agent: Mozilla/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36" ^
  -H "Referer: http://10.255.254.1:801/" ^
  -H "Accept: */*" ^
  -H "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8" > "%RESPONSE_FILE%"

:: 设置循环次数
set "attempt=0"
set "max_attempts=5"

:check_loop
set /a attempt+=1
echo 第 %attempt% 次检测网络状态中...
timeout /t 2 /nobreak >nul

curl -s --head --connect-timeout 3 http://www.baidu.com | findstr /r /c:"HTTP/[0-9]\.[0-9] 200" >nul
if %errorlevel%==0 (
    echo 登录成功，当前已可以上网！
    goto cleanup
)

if %attempt% LSS %max_attempts% (
    goto check_loop
)

echo 参数错误：登录失败，网络连接尝试已达到最大次数限制。

:cleanup
:: 清理临时文件
del "%RESPONSE_FILE%" 2>nul


:end
echo.
echo 【按任意键退出程序...】
pause >nul
