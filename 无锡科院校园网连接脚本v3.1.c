#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <string.h>
#include <conio.h>
#include <Shlobj.h>

// 配置常量（根据实际校园网调整网关IP）
#define CAMPUS_GATEWAY "10.255.254.1"  // 校园网网关核心IP
#define CONFIG_FILE_NAME "无锡科院校园网账密.txt"  // 账密文件名（自定义）
#define WLAN_IP "10.16.41.102"         // 需替换为自己设备的校园网IP
#define WLAN_MAC "000000000000"        // 需替换为自己设备的MAC地址（无空格）
#define MAX_ATTEMPTS 5                  // 登录状态最大检测次数
#define DELAY_SHORT 2000                // 短延时（毫秒，检测间隔）
#define DELAY_LONG 5000                 // 长延时（毫秒，注销后等待）
#define MAX_STR_LEN 100                 // 字符串最大长度（账号/密码）

// 全局变量（存储账号信息）
char id[MAX_STR_LEN], pwd[MAX_STR_LEN], isp[MAX_STR_LEN], uname[MAX_STR_LEN];
char config_file_path[MAX_PATH];  // 存储账密文件的完整路径（桌面路径+文件名）

// 辅助函数：获取桌面完整路径（存入参数path中）
void get_desktop_path(char *path) {
    // SHGetFolderPathA：Windows API，获取系统特殊文件夹路径（CSIDL_DESKTOP=0代表桌面）
    SHGetFolderPathA(NULL, 0, NULL, SHGFP_TYPE_CURRENT, path);
    // 拼接路径分隔符（确保路径格式正确，如"C:\Users\XXX\Desktop\"）
    strcat(path, "\\");
}

// 1. 检测是否连接校园网（ping网关，返回1=是，0=否）
int check_campus_net() {
    char cmd[256];
    // ping网关1次，超时2秒，结果重定向到nul（不显示ping过程）
    sprintf(cmd, "ping -n 1 -w 2000 %s >nul", CAMPUS_GATEWAY);
    return system(cmd) == 0 ? 1 : 0;
}

// 2. 检测网络是否可上网（返回1=已联网，0=未联网）
int check_network() {
    char cmd[256];
    // curl请求百度首页，仅获取响应头，超时3秒，筛选200状态码（成功标志）
    sprintf(cmd, "curl -s --head --connect-timeout 3 http://www.baidu.com | findstr /r /c:\"HTTP/[0-9]\\.[0-9] 200\" >nul");
    return system(cmd) == 0 ? 1 : 0;
}

// 3. 保存账号信息到桌面（无锡科院校园网账密.txt）
void save_config() {
    // 打开文件（"w"：写入模式，若文件不存在则创建，存在则覆盖）
    FILE *fp = fopen(config_file_path, "w");
    if (!fp) {  // 打开失败（如权限不足）
        printf("账号保存失败！请检查桌面权限\n");
        return;
    }
    // 写入账号信息（学号、密码、运营商，每行一个）
    fprintf(fp, "%s\n%s\n%s\n", id, pwd, isp);
    fclose(fp);
    // 提示保存成功及文件路径
    printf("账号已保存到桌面：%s\n\n", config_file_path);
}

// 4. 从桌面读取账号信息（返回1=成功，0=失败）
int read_config() {
    // 检查文件是否存在（"r"：只读模式，不存在则返回NULL）
    FILE *fp = fopen(config_file_path, "r");
    if (!fp) return 0;
    
    // 读取学号、密码、运营商（若任一读取失败，判定为文件损坏）
    if (!fgets(id, MAX_STR_LEN, fp) || !fgets(pwd, MAX_STR_LEN, fp) || !fgets(isp, MAX_STR_LEN, fp)) {
        fclose(fp);
        return 0;
    }
    
    // 去除每行末尾的换行符（fgets会读取换行符，需手动删除）
    id[strcspn(id, "\n")] = '\0';
    pwd[strcspn(pwd, "\n")] = '\0';
    isp[strcspn(isp, "\n")] = '\0';
    // 拼接完整账号（如"2023001@cmcc"，用于登录请求）
    sprintf(uname, "%s@%s", id, isp);
    
    fclose(fp);
    return 1;
}

// 5. 手动输入账号信息（密码隐藏显示）
void input_account() {
    int choice;
    // 输入学号
    printf("请输入学号：");
    scanf("%s", id);
    getchar();  // 吸收scanf后的换行符，避免影响后续_getch()
    
    // 输入密码（隐藏显示，按回车结束，支持退格）
    printf("请输入校园网密码：");
    int i = 0;
    char ch;
    while ((ch = _getch()) != '\r') {  // _getch()：无回显获取按键
        if (ch == 8 && i > 0) {        // 按下退格键（ASCII=8）
            printf("\b \b");           // 光标回退+清除字符+光标再回退
            i--;
        } else if (ch != 8) {          // 非退格键（正常字符）
            pwd[i++] = ch;
            printf("*");               // 显示为*，隐藏真实密码
        }
    }
    pwd[i] = '\0';  // 手动添加字符串结束符
    printf("\n");
    
    // 选择运营商（1=移动/2=联通→cmcc，3=电信→telecom）
    printf("\n请选择运营商（输入数字）：\n");
    printf("1、中国移动\n");
    printf("2、中国联通\n");
    printf("3、中国电信\n");
    printf("你的选择：");
    while (1) {
        scanf("%d", &choice);
        if (choice >= 1 && choice <= 3) break;  // 仅允许输入1-3
        printf("输入错误！请重新选择（1-3）：");
    }
    // 根据选择设置运营商标识
    if (choice == 1 || choice == 2) {
        strcpy(isp, "cmcc");
    } else {
        strcpy(isp, "telecom");
    }
    // 拼接完整账号
    sprintf(uname, "%s@%s", id, isp);
    // 保存账号到桌面
    save_config();
}

// 6. 查看已保存的账号信息
void show_config() {
    if (!read_config()) {  // 读取失败（无文件或文件损坏）
        printf("未检测到已保存的账号！\n");
        return;
    }
    // 格式化输出账号信息
    printf("\n=== 已保存账号信息 ===\n");
    printf("学号：%s\n", id);
    printf("密码：%s\n", pwd);
    printf("运营商：%s\n", strcmp(isp, "cmcc") ? "中国电信" : "中国移动/中国联通");
    printf("完整账号：%s\n", uname);
    printf("账密文件路径：%s\n\n", config_file_path);
}

// 主函数（核心逻辑：环境检测→账号处理→登录/注销→结果判断）
int main() {
    // 设置控制台编码为GBK（解决中文路径/提示乱码问题）
    SetConsoleOutputCP(936);
    
    char login_url[512], logout_url[512], unbind_url[512];
    char temp_file[256], cmd[1024];
    time_t now;
    struct tm *t;
    int attempt, net_status;

    // 初始化：获取桌面路径→拼接账密文件完整路径
    char desktop_path[MAX_PATH];
    get_desktop_path(desktop_path);
    sprintf(config_file_path, "%s%s", desktop_path, CONFIG_FILE_NAME);

    // 初始化临时文件路径（系统临时目录，存储登录响应日志，避免残留）
    sprintf(temp_file, "%s\\login_response.txt", getenv("TEMP"));

    // 打印当前时间（便于排查登录时间和问题）
    time(&now);
    t = localtime(&now);
    printf("==============================================\n");
    printf("[%04d-%02d-%02d %02d:%02d:%02d] 无锡科院校园网登录\n", 
           t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
           t->tm_hour, t->tm_min, t->tm_sec);
    printf("==============================================\n\n");

    // 第一步：检测是否为校园网（非校园网直接退出）
    printf("正在检测是否连接校园网...\n");
    if (!check_campus_net()) {
        printf("未检测到校园网环境，脚本即将关闭\n");
        Sleep(1500);  // 延时1.5秒，让用户看清提示
        return 0;
    }
    printf("已确认校园网环境\n\n");

    // 第二步：读取账号（有则自动使用，无则手动输入）
    if (read_config()) {
        printf("已检测到账号，自动开始登录：%s\n", uname);
    } else {
        printf("未检测到账号，需手动输入\n");
        input_account();
    }

    // 第三步：检测网络状态（已联网则先注销+解绑，避免多设备登录冲突）
    printf("正在检测网络连通性...\n");
    net_status = check_network();
    if (net_status) {
        printf("当前已联网，执行注销操作...\n");
        // 构造注销请求URL（校园网Portal接口标准格式）
        sprintf(logout_url, "http://%s:801/eportal/?c=Portal&a=logout&callback=dr1004&login_method=1&user_account=drcom&user_password=123&ac_logout=1&register_mode=1&wlan_user_ip=%s&wlan_user_ipv6=&wlan_vlan_id=1&wlan_user_mac=%s&wlan_ac_ip=&wlan_ac_name=&jsVersion=3.3.2&v=850",
                CAMPUS_GATEWAY, WLAN_IP, WLAN_MAC);
        // 执行注销请求（curl无回显，结果存入临时文件）
        sprintf(cmd, "curl -s \"%s\" -H \"User-Agent: Mozilla/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36\" -H \"Referer: http://%s:801/\" -H \"Accept: */*\" -H \"Accept-Language: zh-CN,zh;q=0.9,en;q=0.8\" > \"%s\"",
                logout_url, CAMPUS_GATEWAY, temp_file);
        system(cmd);
        Sleep(DELAY_LONG);  // 注销后等待5秒，确保状态清除

        printf("正在解除MAC绑定...\n");
        // 构造MAC解绑请求URL（避免设备绑定导致登录失败）
        sprintf(unbind_url, "http://%s:801/eportal/?c=Portal&a=unbind_mac&callback=dr1003&user_account=%s&wlan_user_mac=%s&wlan_user_ip=%s&jsVersion=3.3.2&v=8416",
                CAMPUS_GATEWAY, uname, WLAN_MAC, WLAN_IP);
        // 执行解绑请求
        sprintf(cmd, "curl -s \"%s\" -H \"User-Agent: Mozilla/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36\" -H \"Referer: http://%s:801/\" -H \"Accept: */*\" -H \"Accept-Language: zh-CN,zh;q=0.9,en;q=0.8\" > \"%s\"",
                unbind_url, CAMPUS_GATEWAY, temp_file);
        system(cmd);
        Sleep(DELAY_SHORT);  // 解绑后等待2秒
    }

    // 第四步：发送登录请求（核心步骤）
    printf("正在用账号【%s】登录...\n", uname);
    // 构造登录请求URL（需匹配校园网Portal接口参数，不同学校可能需调整v参数）
    sprintf(login_url, "http://%s:801/eportal/?c=Portal&a=login&callback=dr1003&login_method=1&user_account=,0,%s&user_password=%s&wlan_user_ip=%s&wlan_user_ipv6=&wlan_user_mac=000000000000&wlan_user_ac_ip=&wlan_ac_name=&jsVersion=3.3.2&v=9850",
            CAMPUS_GATEWAY, uname, pwd, WLAN_IP);
    // 执行登录请求（模拟浏览器UA，避免被接口拦截）
    sprintf(cmd, "curl -s \"%s\" -H \"User-Agent: Mozilla/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36\" -H \"Referer: http://%s:801/\" -H \"Accept: */*\" -H \"Accept-Language: zh-CN,zh;q=0.9,en;q=0.8\" > \"%s\"",
            login_url, CAMPUS_GATEWAY, temp_file);
    system(cmd);

    // 第五步：检测登录结果（最多检测5次，每次间隔2秒）
    for (attempt = 1; attempt <= MAX_ATTEMPTS; attempt++) {
        printf("第%d次检测登录状态...\n", attempt);
        Sleep(DELAY_SHORT);
        if (check_network()) {  // 检测到可上网→登录成功
            printf("\n登录成功！可正常上网\n");
            goto cleanup;  // 跳转到清理步骤（删除临时文件）
        }
    }

    // 登录失败（已达最大检测次数）
    printf("\n登录失败！可能原因：\n");
    printf("1. 账号密码错误\n");
    printf("2. WLAN_IP或WLAN_MAC配置错误\n");
    printf("3. 校园网接口参数变更\n\n");

    // 清理步骤：删除临时文件（避免占用空间）
cleanup:
    remove(temp_file);

    // 窗口保留5秒自动关闭，期间按任意键可查看账号
    printf("=== 窗口将在5秒后自动关闭 ===\n");
    printf("按任意键可查看已保存账号...\n");
    for (int i = 5; i > 0; i--) {
        printf("剩余 %d 秒...\n", i);
        if (_kbhit()) {  // 检测到按键
            show_config();
            printf("按任意键退出...\n");
            system("pause >nul");  // 等待按键（不显示"请按任意键继续..."）
            return 0;
        }
        Sleep(1000);  // 每秒刷新一次倒计时
    }

    return 0;
}