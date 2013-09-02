#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//比较汉字是否相同
int hzcmp(WORDORIGIN hz1, WORDORIGIN hz2)
{
    if(hz1.size == hz2.size)
    {
        return memcmp(hz1.str, hz2.str, hz1.size);
    }
    return -1;
}

gboolean is_same_word(AWORD word1, WORDORIGIN word2)
{
    if(word1.size == word2.size && memcmp(word1.str, word2.str, word1.size) == 0)
    {
        return TRUE;
    }
    return FALSE;
}
//获取绝对路径
char *get_real_path(const char *path, char *out, int size)
{
    if(*path == '/')
        return strncpy(out,path,size);

    char cwd[256];
    getcwd(cwd,256);
    snprintf(out,size,"%s/%s",cwd,path);
    return out;
}

int is_english(char* p)
{
    char c = *p;
    return (c > 64 && c < 91) || ( c > 96 && c < 123);
}

/**
 * 判断是否汉字
 * TODO 汉字在unicode 4E00-9FA5
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
int is_chinese(char* p)
{
    //return is_chinese_gbk(p);
    return is_chinese_utf8(p);
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

char *forward_a_hanzi_gbk(char *p)
{
    return p + 2;
}
char *forward_a_hanzi_utf8(char *p)
{
    return p + get_utf8_size(p);
}
//前进到这个汉字结束
char *forward_a_hanzi(char *p)
{
    return forward_a_hanzi_utf8(p);
}

//对于UTF-8编码中的任意字节B，如果B的第一位为0，则B为ASCII码，并且B独立的表示一个字符;
//如果B的第一位为1，第二位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的一个字节，并且不为字符的第一个字节编码;
//如果B的前两位为1，第三位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的第一个字节，并且该字符由两个字节表示;
//如果B的前三位为1，第四位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的第一个字节，并且该字符由三个字节表示;
//如果B的前四位为1，第五位为0，则B为一个非ASCII字符（该字符由多个字节表示）中的第一个字节，并且该字符由四个字节表示;

//str需在字首
int get_utf8_size(char *p)
{
    if((*p & 0x80) == 0)
    {
        return 1;
    }
    else
    {
        if((*p & 0x40) != 0 && (*p & 0x20) == 0)
        {
            return 2;
        }
        else if((*p & 0x40) != 0 && (*p & 0x20) != 0 && (*p & 0x10) == 0)
        {
            return 3;
        }
        else if((*p & 0x40) != 0 && (*p & 0x20) != 0 && (*p & 0x10) != 0 && (*p & 0x08) == 0)
        {
            return 4;
        }
        else
        {
            printf("not utf8!\n");
            exit(1);
        }
    }

}
char *find_a_hanzi_utf8(char *p, WORDORIGIN *hanzi)
{
    while((*p & 0x80) == 0)
    {
        if(*p == '\0') return NULL;
        p++;
    }

    if(hanzi != NULL)
    {
        hanzi->str = p;
        hanzi->size = get_utf8_size(p);
        if(hanzi->size < 1 || hanzi->size > 4)
        {
            return NULL;
        }
    }

    return p;
}
int is_chinese_gbk(char* p);
char *find_a_hanzi_gbk(char *p, WORDORIGIN *hanzi)
{
    while(!is_chinese_gbk(p))
    {
        if(*p == '\0') return NULL;
        p++;
    }

    if(hanzi != NULL)
    {
        hanzi->str = p;
        hanzi->size = 2;
    }

    return p;
}

//前进到汉字处 返回值为汉字起始位置
char *find_a_hanzi(char *p, WORDORIGIN *hanzi)
{
    return find_a_hanzi_utf8(p,hanzi);
}

//like strdup
char *memdup(char *data, int size)
{
    if(data == NULL)
    {
        return NULL;
    }

    char *p;
    p=malloc(size);
    memcpy(p,data,size);
    return p;
}


int get_word_n_utf8(char *p, int len)
{
    int size = 0;
    int i;
    int n;
    for(i=0; i<len; i++)
    {
        if(*p == '\0')
        {
            return -1;
        }
        n = get_utf8_size(p);
        size += n;
        p += n;
    }
    return size;
}
int get_word_n_gbk(char *p, int len)
{
    char *q = p + 2 * len;
    while(p < q && *p != '\0')
        p++;

    if(p < q)
        return -1;
    else
        return len * 2;
}
// 返回-1表示失败
int get_word_n(char *p, int len)
{
    return get_word_n_utf8(p,len);
}
char *find_the_hanzi(char *p, WORDORIGIN hanzi)
{
    while(*p != '\0' && memcmp(p, hanzi.str, hanzi.size) != 0)
        p++;
    return p;
}
void printword(WORDORIGIN word)
{
    char sword[256];
    bzero(sword, 256);
    memcpy(sword, word.str, word.size);
    printf("%s\n", sword);
}
void printaword(AWORD word)
{
    char sword[256];
    bzero(sword, 256);
    memcpy(sword, word.str, word.size);
    printf("%s\t%d\n", sword, word.freq);
}

void addsep_utf8(char **p)
{
    **p = '.';
    (*p)++;
}
void addsep(char **p)
{
    addsep_utf8(p);
}
