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

const int EN_LENGHT = 10;
int is_chinese(char* p);
int is_english(char* p);
void  write_chinese(char **p, FILE *fp_out);
void tidy(const char *fin_path, const char *fout_path);

int main(int argc, char ** argv)
{
    if(argc != 3)
        return;

    char *fin_path = argv[1];
    char *fout_path = argv[2];

    unsigned long start = clock();

    tidy(fin_path, fout_path);

    printf("%.3lf\n", ((double)(clock() - start))/CLOCKS_PER_SEC);
}

void tidy(const char *fin_path, const char *fout_path)
{
    FILE *fp_in, *fp_out;
    fp_in = fopen(fin_path,"r");
    fp_out = fopen(fout_path,"w");

    if(fp_in == NULL || fp_out == NULL)
    {
        return;
    }

    fseek(fp_in, 0L, SEEK_END);
    int size = ftell(fp_in);
    fseek(fp_in, 0L, SEEK_SET);

    char *buf;
    buf = (char*)malloc(size+1);

    int len = fread(buf,1,size,fp_in);
    buf[len] = '\0';

    char *p = buf;
    char *start = buf;
    // 汉字或汉字之后EN_LENGHT个英文之内保留
    do
    {
        if(is_chinese(p))
        {
            write_chinese(&p, fp_out);
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
                fputc(*p, fp_out);
            }
            p++;
        }
    }while(*p != '\0');
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
void  write_chinese_utf8(char **p, FILE *fp_out)
{
    while(**p & 0x80)
    {
        fwrite(*p,1,1,fp_out);
        *p += 1;
    }
}
void  write_chinese_gbk(char **p, FILE *fp_out)
{
    fwrite(*p,1,2,fp_out);
    *p += 2;
}

void  write_chinese(char **p, FILE *fp_out)
{
    //write_chinese_gbk(p, fp_out);
    write_chinese_utf8(p, fp_out);
}
int is_chinese(char* p)
{
    //return is_chinese_gbk(p);
    return is_chinese_utf8(p);
}
