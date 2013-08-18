#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

#include "common.h"


void analyse(const char *mbuf, int len);
//查询hanzi是否在全局字列表里
gboolean is_hanzi_in_list(WORDORIGIN hanzi);
void add_hanzi_to_list(WORDORIGIN hanzi);
//查找出所有以hanzi开头的词
void find_all_word(char *p, WORDORIGIN hanzi);
void freehzlist();
void freewordlist();

static GList *hanzi_list = NULL;
static GList *word_list = NULL;

static char *buf_end = NULL;

int main(int argc, char ** argv)
{
    if(argc != 2)
        return;

    int fd = open(argv[1],O_RDONLY);
    int len = lseek(fd,0,SEEK_END);
    char *mbuf = (char *) mmap(NULL,len,PROT_READ,MAP_PRIVATE,fd,0);
    buf_end = mbuf + len;

    analyse(mbuf,len);

    freehzlist();
    freewordlist();
    munmap(mbuf, len);
    return 0;
}

void analyse(const char *mbuf, int len)
{
    printf("total:%d\n",len);
    printf("strlen:%d\n",strlen(mbuf));
    char *p = (char *)mbuf;
    do
    {
        //寻找中文字
        WORDORIGIN hanzi;
        p=find_a_hanzi(p,&hanzi);
        if(p==NULL) break;

        //若其不在已处理列表中,则找出所有以其为起始长度为n个的词
        if(!is_hanzi_in_list(hanzi))
        {
            add_hanzi_to_list(hanzi);
            find_all_word(p,hanzi);
        }
        else
        {
            p = forward_a_hanzi(p);
        }
    }while(*p != '\0' && p < buf_end);
}

//查询hanzi是否在全局字列表里
gboolean is_hanzi_in_list(WORDORIGIN hanzi)
{
    GList *node;
    for(node=hanzi_list;node;node=g_list_next(node))
    {
        WORDORIGIN *data = node->data;
        if(hzcmp(*data, hanzi)==0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

void add_hanzi_to_list(WORDORIGIN hanzi)
{
    printword(hanzi);
    WORDORIGIN *data = g_new0(WORDORIGIN, 1);
    memcpy(data, &hanzi, sizeof(WORDORIGIN));
    hanzi_list = g_list_append(hanzi_list,data);
}

AWORD *get_word_in_list(WORDORIGIN word)
{
    GList *node;
    for(node=word_list;node;node=g_list_next(node))
    {
        AWORD *data = node->data;
        if(is_same_word(*data, word))
        {
            return data;
        }
    }
    return NULL;
}
void add_word_to_list(AWORD word)
{
    //printaword(word);
    AWORD *data = g_new0(AWORD, 1);
    memcpy(data, &word, sizeof(AWORD));
    word_list = g_list_append(word_list,data);
}
//查找出所有以hanzi开头的词
//start需是hanzi开头之处
void find_all_word(char *start, WORDORIGIN hanzi)
{
    char *p;
    int i;
    int size;

    p = start;
    while(*p != '\0' && buf_end - p > 1)
    {
        //printf("p:0x%x\n",p);
        for(i=MAX_WORD_LEN; i>1; i--)
        {
            size = get_word_n(p, i);
            if(size == -1) continue;

            WORDORIGIN word = {p, size};
            AWORD * pword;

            if((pword = get_word_in_list(word)) != NULL)
            {
                pword->freq += 1;
            }
            else
            {
                AWORD word = {p,size,1};
                add_word_to_list(word);
            }
        }

        p = forward_a_hanzi(p);
        p = find_the_hanzi(p, hanzi);
    }
}

void freehzlist()
{
    printf("hzlist:%d\n",g_list_length(hanzi_list));
    GList *node;
    for(node=hanzi_list;node;node=g_list_next(node))
    {
        free(node->data);
    }
}
void freewordlist()
{
    printf("wordlist:%d\n",g_list_length(hanzi_list));
    char sword[256];
    AWORD *word;
    GList *node;
    for(node=word_list;node;node=g_list_next(node))
    {
        word = node->data;
        bzero(sword, 256);
        memcpy(sword, word->str, word->size);
        printf("%d\t%s\n", word->freq, sword);
        free(node->data);
    }
}
