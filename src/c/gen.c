#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

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
void find_all_word(char *p, size_t len);
void freehzlist();
void freewordlist();
void freeorphanlist();
void sort_list();
void clean_list();

static GSList *hanzi_list = NULL;
static GSList *word_list = NULL;
static GSList *orphan_list = NULL;

int main(int argc, char ** argv)
{
    if(argc != 2)
        return;

    long long L1,L2,L3,L4;
    struct timeval tv;

    int fd = open(argv[1],O_RDONLY);

    gettimeofday(&tv,NULL);
    L1 = tv.tv_sec*1000*1000 + tv.tv_usec;

    size_t len = lseek(fd,0,SEEK_END);

    gettimeofday(&tv,NULL);
    L2 = tv.tv_sec*1000*1000 + tv.tv_usec;

    char *mbuf = (char *) mmap(NULL,len,PROT_READ,MAP_PRIVATE,fd,0);

    gettimeofday(&tv,NULL);
    L3 = tv.tv_sec*1000*1000 + tv.tv_usec;

    printf("seek:%lldus\nmmap:%lldus\n",L2-L1, L3-L2);

    printf("start find word:\n");
    find_all_word(mbuf, len);

    gettimeofday(&tv,NULL);
    L1 = tv.tv_sec*1000*1000 + tv.tv_usec;

    printf("find_all_word:%lldus\n",L1-L3);

    //// TODO 存在包含关系且频率(一样)的词只保留最长的
    //tidy_list();

    ////总共只出现1次的不算词 不保留
    clean_list();

    gettimeofday(&tv,NULL);
    L2 = tv.tv_sec*1000*1000 + tv.tv_usec;

    //按词频排序
    sort_list();

    gettimeofday(&tv,NULL);
    L3 = tv.tv_sec*1000*1000 + tv.tv_usec;

    freehzlist();
    freewordlist();
    freeorphanlist();

    gettimeofday(&tv,NULL);
    L4 = tv.tv_sec*1000*1000 + tv.tv_usec;
    printf("clean:%lldus\nsort:%lldus\nfree:%lldus\n",L2-L1, L4-L3);
    munmap(mbuf, len);
    return 0;
}

//查询hanzi是否在全局字列表里
gboolean is_hanzi_in_list(WORDORIGIN hanzi)
{
    GSList *node;
    for(node=hanzi_list;node;node=g_slist_next(node))
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
    hanzi_list = g_slist_append(hanzi_list,data);
}

AWORD *get_word_in_list(WORDORIGIN word)
{
    GSList *node;
    for(node=word_list;node;node=g_slist_next(node))
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
    word_list = g_slist_append(word_list,data);
}
//查找出所有以hanzi开头的词
//start需是hanzi开头之处
//TODO 使用单向链表来优化查询
//TODO 使用树来优化查询
void find_all_word(char *start, size_t len)
{
    long long L1,L2,L3,L4,L5;
    long long M1,M2,M3,M4,M5;
    struct timeval tv;

    char *p;
    int i;
    int size;

    size_t tip_pos = 0;
    size_t step = len/100;

    p = start;
    while(*p != '\0')
    {
//    gettimeofday(&tv,NULL);
//    L1 = tv.tv_sec*1000*1000 + tv.tv_usec;
        p = find_a_hanzi(p, NULL);
        if(p == NULL)
            break;

//    gettimeofday(&tv,NULL);
//    L2 = tv.tv_sec*1000*1000 + tv.tv_usec;
        if(p - start > tip_pos + step)
        {
            tip_pos = (p - start) / 100 * 100;
            printf("\r%d%% ...", (p - start) / step);
            fflush(stdout);
        }

//    gettimeofday(&tv,NULL);
//    L3 = tv.tv_sec*1000*1000 + tv.tv_usec;
        for(i=MAX_WORD_LEN; i>1; i--)
        {
//    gettimeofday(&tv,NULL);
//    M1 = tv.tv_sec*1000*1000 + tv.tv_usec;
            size = get_word_n(p, i);
            if(size == -1) continue;

            WORDORIGIN word = {p, size};
            AWORD * pword;

//    gettimeofday(&tv,NULL);
//    M2 = tv.tv_sec*1000*1000 + tv.tv_usec;
            if((pword = get_word_in_list(word)) != NULL)
            {
//    gettimeofday(&tv,NULL);
//    M3 = tv.tv_sec*1000*1000 + tv.tv_usec;
                pword->freq += 1;
            }
            else
            {
//    gettimeofday(&tv,NULL);
//    M3 = tv.tv_sec*1000*1000 + tv.tv_usec;
                AWORD word = {p,size,1};
                add_word_to_list(word);
            }
//    gettimeofday(&tv,NULL);
//    M4 = tv.tv_sec*1000*1000 + tv.tv_usec;
        }
//    gettimeofday(&tv,NULL);
//    L4 = tv.tv_sec*1000*1000 + tv.tv_usec;

        p = forward_a_hanzi(p);
//    gettimeofday(&tv,NULL);
//    L5 = tv.tv_sec*1000*1000 + tv.tv_usec;
//    printf(":%lldus:%lldus:%lldus:%lldus\n",L2-L1, L3-L2,L4-L3, L5-L4);
//    printf(":%lldus:%lldus:%lldus\n",M2-M1, M3-M2,M4-M3);
    }
    printf("\r100%%.\n");
}

void freehzlist()
{
    printf("hzlist:%d\n",g_slist_length(hanzi_list));
    GSList *node;
    for(node=hanzi_list;node;node=g_slist_next(node))
    {
        free(node->data);
    }
}
void freewordlist()
{
    printf("wordlist:%d\n",g_slist_length(word_list));
    int fd = open("/tmp/wordlist",O_WRONLY|O_CREAT|O_TRUNC,0600);
    char buf[512];
    char sword[256];
    AWORD *word;
    GSList *node;
    for(node=word_list;node;node=g_slist_next(node))
    {
        word = node->data;
        bzero(sword, 256);
        memcpy(sword, word->str, word->size);
        snprintf(buf, 512, "%d\t%s\n", word->freq, sword);
        write(fd, buf, strlen(buf));
        free(node->data);
    }
    close(fd);
}
void freeorphanlist()
{
    printf("orphanlist:%d\n",g_slist_length(orphan_list));
    int fd = open("/tmp/orphanlist",O_WRONLY|O_CREAT|O_TRUNC,0600);
    char buf[512];
    char sword[256];
    AWORD *word;
    GSList *node;
    for(node=orphan_list;node;node=g_slist_next(node))
    {
        word = node->data;
        bzero(sword, 256);
        memcpy(sword, word->str, word->size);
        snprintf(buf, 512, "%d\t%s\n", word->freq, sword);
        write(fd, buf, strlen(buf));
        free(node->data);
    }
    close(fd);
}

gint compare_word(gconstpointer a, gconstpointer b)
{
    AWORD *w1 = (AWORD *)a;
    AWORD *w2 = (AWORD *)b;
    return w1->freq - w2->freq;
}

void sort_list()
{
    word_list = g_slist_sort(word_list, compare_word);
}

void clean_list()
{
    AWORD *word;
    GSList *node;
    for(node=word_list;node;node=g_slist_next(node))
    {
        word = node->data;
        if(word->freq == 1)
        {
            word_list = g_slist_remove(word_list, word);
            free(word);
        }
        else if(g_utf8_strlen(word->str, word->size) == MAX_WORD_LEN)
        {
            word_list = g_slist_remove(word_list, word);
            orphan_list = g_slist_append(orphan_list, word);
            //TODO research orphan
        }
    }
}

