/*
 * 程序名： C语言遍历目录并输出目录内所有文件名
 * 作者：        Yucoat(www.yucoat.com)
 * 时间：        2011-10-5
 * 描述：        遍历目录，输出非隐藏文件的文件名并遍历目录内的子目录
 */
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

/*输出空格，为了能够让输出结果用层次感，使用这个函数输出空格，其中deepth代表层*/
void printfspace(int deepth);

/*遍历函数directory代表目录的路径，deepth代表目录的深度*/
void scan_dir(const char *directory, int deepth);

/*主函数*/
int main(int argc, char *argv[])
{
    int deepth = 0;
    if (argc != 2)
    {
        printf("Error!\nUsage:%s path\n", argv[0]);
        return 1;
    }
    scan_dir(argv[1], deepth);
    return 0;
}


void printfspace(int deepth)
{
    int temp = 0;
    while (temp < deepth)
    {
        printf("    ");
        temp++;
    }
}


void scan_dir(const char *directory, int deepth)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    char buf[256];

    if((dp = opendir(directory)) == NULL)
    {
        perror("opendir");
        return;
    }
    chdir(directory);

    while ((entry = readdir(dp)) != NULL)
    {
        lstat(entry->d_name, &statbuf);
        if (S_ISDIR(statbuf.st_mode))
        {

            if ((strcmp(entry->d_name, ".") != 0) &&
                    (strcmp(entry->d_name, "..") != 0) &&
                    (entry->d_name[0] != '.'))
            {
                printfspace(deepth);
                printf("%s\n", entry->d_name);
                scan_dir(entry->d_name, (deepth+1));
            }
        }
        else
        {
            if (entry->d_name[0] != '.')
            {
                printfspace(deepth);
                getcwd(buf,256);
                printf("%s/%s\n", buf, entry->d_name);
            }
        }
    }
    chdir("..");
    closedir(dp);
}
