#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

#include "common.h"


//查询hanzi是否在全局字列表里
gboolean is_hanzi_in_list(WORDORIGIN hanzi);
void add_hanzi_to_list(WORDORIGIN hanzi);
//查找出所有以hanzi开头的词
void find_all_word(char *p);
void freehzlist();
void freewordlist();
void sort_list();
void clean_list();

static GList *hanzi_list = NULL;
static GList *word_list = NULL;
static GList *orphan_list = NULL;

int main(int argc, char ** argv)
{
    if(argc != 2)
        return;

    int fd = open(argv[1],O_RDONLY);
    int len = lseek(fd,0,SEEK_END);
    char *mbuf = (char *) mmap(NULL,len,PROT_READ,MAP_PRIVATE,fd,0);

    find_all_word(mbuf);

    //// TODO 存在包含关系且频率(一样)的词只保留最长的
    //tidy_list();

    ////总共只出现1次的不算词 不保留
    clean_list();

    //按词频排序
    sort_list();

    freehzlist();
    freewordlist();
    munmap(mbuf, len);
    return 0;
}

//查询hanzi是否在全局字列表里
gboolean is_hanzi_in_list(WORDORIGIN hanzi)
{
    GList *node;
    for(node=g_list_first(hanzi_list);node;node=g_list_next(node))
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
    for(node=g_list_first(word_list);node;node=g_list_next(node))
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
void find_all_word(char *start)
{
    char *p;
    int i;
    int size;

    p = start;
    while(*p != '\0')
    {
        //printf("p:0x%x\n",p);
        p = find_a_hanzi(p, NULL);
        if(p == NULL)
            break;

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
    }
}

void freehzlist()
{
    printf("hzlist:%d\n",g_list_length(hanzi_list));
    GList *node;
    for(node=g_list_first(hanzi_list);node;node=g_list_next(node))
    {
        free(node->data);
    }
}
void freewordlist()
{
    //TODO write to file
    printf("wordlist:%d\n",g_list_length(hanzi_list));
    char sword[256];
    AWORD *word;
    GList *node;
    for(node=g_list_first(word_list);node;node=g_list_next(node))
    {
        word = node->data;
        bzero(sword, 256);
        memcpy(sword, word->str, word->size);
        printf("%d\t%s\n", word->freq, sword);
        free(node->data);
    }
}

gint compare_word(gconstpointer a, gconstpointer b)
{
    AWORD *w1 = (AWORD *)a;
    AWORD *w2 = (AWORD *)b;
    return w1->freq - w2->freq;
}

void sort_list()
{
    word_list = g_list_sort(word_list, compare_word);
}

void clean_list()
{
    AWORD *word;
    GList *node;
    for(node=g_list_first(word_list);node;node=g_list_next(node))
    {
        word = node->data;
        if(word->freq == 1)
        {
            word_list = g_list_delete_link(word_list, node);
            free(word);
        }
        else if(g_utf8_strlen(word->str, word->size) == MAX_WORD_LEN)
        {
            word_list = g_list_delete_link(word_list, node);
            orphan_list = g_list_append(orphan_list, word);
            //TODO research orphan
            //TODO write to file
        }
    }
}

