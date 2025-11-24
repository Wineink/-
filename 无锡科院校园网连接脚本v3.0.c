#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <string.h>
#include <conio.h>

// 配置常量（根据实际校园网调整网关IP）
#define CAMPUS_GATEWAY "10.255.254.1"  // 校园网网关核心IP
#define CONFIG_FILE "campus_net_cfg.txt"
#define WLAN_IP "10.16.41.102"
#define WLAN_MAC "000000000000"
#define MAX_ATTEMPTS 5
#define DELAY_SHORT 2000
#define DELAY_LONG 5000
#define MAX_STR_LEN 100

// 全局变量
char id[MAX_STR_LEN], pwd[MAX_STR_LEN], isp[MAX_STR_LEN], uname[MAX_STR_LEN];

// 1. 检测是否连接校园网（ping网关，返回1=是，0=否）
int check_campus_net() {
    char cmd[256];
    sprintf(cmd, "ping -n 1 -w 2000 %s >nul", CAMPUS_GATEWAY);
    return system(cmd) == 0 ? 1 : 0;
}

// 2. 检测网络是否可上网（返回1=已联网，0=未联网）
int check_network() {
    char cmd[256];
    sprintf(cmd, "curl -s --head --connect-timeout 3 http://www.baidu.com | findstr /r /c:\"HTTP/[0-9]\\.[0-9] 200\" >nul");
    return system(cmd) == 0 ? 1 : 0;
}

// 3. 保存账号信息
void save_config() {
    FILE *fp = fopen(CONFIG_FILE, "w");
    if (!fp) {
        printf("账号保存失败！\n");
        return;
    }
    fprintf(fp, "%s\n%s\n%s\n", id, pwd, isp);
    fclose(fp);
    printf("账号已保存到：%s\n\n", CONFIG_FILE);
}

// 4. 读取账号信息（返回1=成功，0=失败）
int read_config() {
    FILE *fp = fopen(CONFIG_FILE, "r");
    if (!fp) return 0;
    
    if (!fgets(id, MAX_STR_LEN, fp) || !fgets(pwd, MAX_STR_LEN, fp) || !fgets(isp, MAX_STR_LEN, fp)) {
        fclose(fp);
        return 0;
    }
    
    id[strcspn(id, "\n")] = '\0';
    pwd[strcspn(pwd, "\n")] = '\0';
    isp[strcspn(isp, "\n")] = '\0';
    sprintf(uname, "%s@%s", id, isp);
    
    fclose(fp);
    return 1;
}

// 5. 输入账号信息
void input_account() {
    int choice;
    printf("请输入学号：");
    scanf("%s", id);
    getchar();
    
    printf("请输入校园网密码：");
    int i = 0;
    char ch;
    while ((ch = _getch()) != '\r') {
        if (ch == 8 && i > 0) {
            printf("\b \b");
            i--;
        } else if (ch != 8) {
            pwd[i++] = ch;
            printf("*");
        }
    }
    pwd[i] = '\0';
    printf("\n");
    
    printf("\n请选择运营商（输入数字）：\n");
    printf("1、中国移动\n");
    printf("2、中国联通\n");
    printf("3、中国电信\n");
    printf("你的选择：");
    while (1) {
        scanf("%d", &choice);
        if (choice >= 1 && choice <= 3) break;
        printf("输入错误！请重新选择（1-3）：");
    }
    choice == 1 || choice == 2 ? strcpy(isp, "cmcc") : strcpy(isp, "telecom");
    sprintf(uname, "%s@%s", id, isp);
    save_config();
}

// 6. 查看账号信息
void show_config() {
    if (!read_config()) {
        printf("未检测到已保存的账号！\n");
        return;
    }
    printf("\n=== 已保存账号信息 ===\n");
    printf("学号：%s\n", id);
    printf("密码：%s\n", pwd);
    printf("运营商：%s\n", strcmp(isp, "cmcc") ? "中国电信" : "中国移动");
    printf("完整账号：%s\n\n", uname);
}

// 主函数
int main() {
    char login_url[512], logout_url[512], unbind_url[512];
    char temp_file[256], cmd[1024];
    time_t now;
    struct tm *t;
    int attempt, net_status;

    // 初始化临时文件路径
    sprintf(temp_file, "%s\\login_response.txt", getenv("TEMP"));

    // 打印当前时间
    time(&now);
    t = localtime(&now);
    printf("==============================================\n");
    printf("[%04d-%02d-%02d %02d:%02d:%02d] 校园网登录\n", 
           t->tm_year+1900, t->tm_mon+1, t->tm_mday,
           t->tm_hour, t->tm_min, t->tm_sec);
    printf("==============================================\n\n");

    // 核心：检测是否为校园网（非校园网直接关闭）
    printf("正在检测是否连接校园网...\n");
    if (!check_campus_net()) {
        printf("未检测到校园网环境，脚本即将关闭\n");
        Sleep(1500);  // 短暂延时（1.5秒）让用户看清提示，无额外按键退出
        return 0;
    }
    printf("已确认校园网环境\n\n");

    // 读取账号：有则直接运行，无则输入
    if (read_config()) {
        printf("已检测到账号，自动开始登录：%s\n", uname);
    } else {
        printf("未检测到账号，需手动输入\n");
        input_account();
    }

    // 检测网络状态：已联网则注销+解绑
    printf("正在检测网络连通性...\n");
    net_status = check_network();
    if (net_status) {
        printf("当前已联网，执行注销操作...\n");
        sprintf(logout_url, "http://%s:801/eportal/?c=Portal&a=logout&callback=dr1004&login_method=1&user_account=drcom&user_password=123&ac_logout=1&register_mode=1&wlan_user_ip=%s&wlan_user_ipv6=&wlan_vlan_id=1&wlan_user_mac=%s&wlan_ac_ip=&wlan_ac_name=&jsVersion=3.3.2&v=850",
                CAMPUS_GATEWAY, WLAN_IP, WLAN_MAC);
        sprintf(cmd, "curl -s \"%s\" -H \"User-Agent: Mozilla/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36\" -H \"Referer: http://%s:801/\" -H \"Accept: */*\" -H \"Accept-Language: zh-CN,zh;q=0.9,en;q=0.8\" > \"%s\"",
                logout_url, CAMPUS_GATEWAY, temp_file);
        system(cmd);
        Sleep(DELAY_LONG);

        printf("正在解除MAC绑定...\n");
        sprintf(unbind_url, "http://%s:801/eportal/?c=Portal&a=unbind_mac&callback=dr1003&user_account=%s&wlan_user_mac=%s&wlan_user_ip=%s&jsVersion=3.3.2&v=8416",
                CAMPUS_GATEWAY, uname, WLAN_MAC, WLAN_IP);
        sprintf(cmd, "curl -s \"%s\" -H \"User-Agent: Mozilla/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36\" -H \"Referer: http://%s:801/\" -H \"Accept: */*\" -H \"Accept-Language: zh-CN,zh;q=0.9,en;q=0.8\" > \"%s\"",
                unbind_url, CAMPUS_GATEWAY, temp_file);
        system(cmd);
        Sleep(DELAY_SHORT);
    }

    // 发送登录请求
    printf("正在用账号【%s】登录...\n", uname);
    sprintf(login_url, "http://%s:801/eportal/?c=Portal&a=login&callback=dr1003&login_method=1&user_account=,0,%s&user_password=%s&wlan_user_ip=%s&wlan_user_ipv6=&wlan_user_mac=000000000000&wlan_user_ac_ip=&wlan_ac_name=&jsVersion=3.3.2&v=9850",
            CAMPUS_GATEWAY, uname, pwd, WLAN_IP);
    sprintf(cmd, "curl -s \"%s\" -H \"User-Agent: Mozilla/537.36 (KHTML, like Gecko) Chrome/142.0.0.0 Safari/537.36\" -H \"Referer: http://%s:801/\" -H \"Accept: */*\" -H \"Accept-Language: zh-CN,zh;q=0.9,en;q=0.8\" > \"%s\"",
            login_url, CAMPUS_GATEWAY, temp_file);
    system(cmd);

    // 检测登录结果
    for (attempt = 1; attempt <= MAX_ATTEMPTS; attempt++) {
        printf("第%d次检测登录状态...\n", attempt);
        Sleep(DELAY_SHORT);
        if (check_network()) {
            printf("\n登录成功！可正常上网\n");
            goto cleanup;
        }
    }

    // 登录失败提示
    printf("\n登录失败！已达最大检测次数\n");

    // 清理临时文件
cleanup:
    remove(temp_file);

    // 校园网环境下：窗口保留5秒，支持按键查询账号
    printf("\n=== 窗口将在5秒后自动关闭 ===\n");
    printf("按任意键可查看已保存账号...\n");
    for (int i = 5; i > 0; i--) {
        printf("剩余 %d 秒...\n", i);
        if (_kbhit()) {
            show_config();
            printf("按任意键退出...\n");
            system("pause >nul");
            return 0;
        }
        Sleep(1000);
    }

    return 0;
}