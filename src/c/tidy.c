/**
 * 整理文章 把汉字无关的都删掉
 * usage: ./tidyfile /path/to/filein /path/to/fileout
 *
 * 效果:
 * 中文后n个字符之外的非中文字符全部删掉
 *
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int EN_LENGHT = 10;
int is_chinese(char* p);
int is_english(char* p);
void  write_chinese(char **p, char **p2);
char *create_tidy_str(const char *fin_path, char **pbuf, int* plen);

int main(int argc, char ** argv)
{
    if(argc != 3)
        return;

    char *fin_path = argv[1];
    char *fout_path = argv[2];

    unsigned long start = clock();

    char *buf;
    int len;
    create_tidy_str(fin_path,&buf,&len);
    //printf("len:%s\n",fin_path);
    //printf("len:%s\n",fout_path);
    //printf("len:%d\n",len);

    FILE *fp_out;
    fp_out = fopen(fout_path,"w");
    if(fp_out == NULL)
    {
        return;
    }

    fwrite(buf,1,len,fp_out);
    fclose(fp_out);
    free(buf);

    printf("%.3lf\n", ((double)(clock() - start))/CLOCKS_PER_SEC);
}

char *create_tidy_str(const char *fin_path, char **pbuf, int* plen)
{
    FILE *fp_in, *fp_out;
    fp_in = fopen(fin_path,"r");

    if(fp_in == NULL)
    {
        return;
    }

    fseek(fp_in, 0L, SEEK_END);
    int size = ftell(fp_in);
    fseek(fp_in, 0L, SEEK_SET);

    char *buf,*buf2;
    buf = (char*)malloc(size+1);
    buf2 = (char*)malloc(size+1);
    *plen = size+1;

    int len = fread(buf,1,size,fp_in);
    buf[len] = '\0';

    char *p = buf;
    char *p2 = buf2;
    char *start = buf;
    // 汉字或汉字之后EN_LENGHT个英文之内保留
    do
    {
        if(is_chinese(p))
        {
            write_chinese(&p, &p2);
            start = p;
        }
        else
        {
            // 非英文不保留,英文则保留有限位数
            if(!is_english(p))
            {
                start = buf;
            }
            else if(p - start <= EN_LENGHT)
            {
                *p2++ = *p;
            }
            p++;
        }
    }while(*p != '\0');

    free(buf);
    *pbuf = buf2;
    fclose(fp_in);
    return *pbuf;
}

int is_english(char* p)
{
    char c = *p;
    return (c > 64 && c < 91) || ( c > 96 && c < 123);
}

/**
 * 判断是否汉字
 *
 * 各种编码判断方式不一样
 */
int is_chinese_utf8(char* p)
{
    // 只是判断了是否是utf8编码
    return *p & 0x80;
}
int is_chinese_gbk(char* p)
{
    char c = *p;
    short high, low;
    unsigned int code;

    if(*p < 0)
    {
        high = (short)(*p + 256);
        low = (short)(*(p+1)+256);
        code = high*256 + low;
        return code>=0xB0A1 && code<=0xF7FE || code>=0x8140 && code<=0xA0FE || code>=0xAA40 && code<=0xFEA0;
    }
    else
    {
        return 0;
    }
}

/**
 * 写入汉字
 *
 * 各种编码写入长度不一样
 */
void  write_chinese_utf8(char **p, char **p2)
{
    while(**p & 0x80)
    {
        **p2 = **p;
        (*p)++;
        (*p2)++;
    }
}
void  write_chinese_gbk(char **p, char **p2)
{
    memcpy(*p2,*p,2);
    *p += 2;
    *p2 += 2;
}

void  write_chinese(char **p, char **p2)
{
    //write_chinese_gbk(p, p2);
    write_chinese_utf8(p, p2);
}
int is_chinese(char* p)
{
    //return is_chinese_gbk(p);
    return is_chinese_utf8(p);
}
