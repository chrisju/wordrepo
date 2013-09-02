#ifndef _COMMON_H_
#define _COMMON_H_

#include <glib.h>

#define MAX_WORD_LEN 5
#define EN_LENGHT 10

//优化:内存池
typedef struct
{
    char *str; //不可修改内容,无需释放
    int size;
    int freq;
} AWORD;
typedef struct
{
    char *str; //不可修改内容,无需释放
    int size; //大小 单位byte
}WORDORIGIN;

int hzcmp(WORDORIGIN hz1, WORDORIGIN hz2);
gboolean is_same_word(AWORD word1, WORDORIGIN word2);
//前进到这个汉字结束
char *forward_a_hanzi(char *p);
//返回值为汉字起始位置
char *find_a_hanzi(char *p, WORDORIGIN *hanzi);
int is_chinese(char* p);
int is_english(char* p);
void  write_chinese(char **p, char **p2);
char *get_real_path(const char *path, char *out, int size);
char *memdup(char *data, int size);
int get_word_n(char *p, int len);
char *find_the_hanzi(char *p, WORDORIGIN hanzi);
void printaword(AWORD word);
void printword(WORDORIGIN word);
void addsep(char **p);

#endif

