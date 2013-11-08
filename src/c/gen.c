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
void freewordtree();
void freeresultlist();
void freeorphanlist();
gint compare_word(gconstpointer a, gconstpointer b);
static gint
word_compare(gconstpointer p1, gconstpointer p2, gpointer data);
static gint
clean_traverse(gpointer key, gpointer value, gpointer data);

static GSList *hanzi_list = NULL;
static GTree *word_tree = NULL;
static GSList *result_list = NULL;
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

    // 查找所有词
    printf("start find word:\n");
    word_tree = g_tree_new_full(word_compare,NULL,free,free);
    find_all_word(mbuf, len);

    gettimeofday(&tv,NULL);
    L1 = tv.tv_sec*1000*1000 + tv.tv_usec;

    printf("find_all_word:%lldus\n",L1-L3);

    // TODO hashtab
    // TODO 清理树1 去除频率不足的词并生成待重搜的词链表
    // TODO 清理树2 过滤掉被包含的子词,生成结果链表
    //不保留长度最长的词,进行进一步处理
    printf("wordtree:%d\n",g_tree_nnodes(word_tree));
    g_tree_foreach(word_tree, clean_traverse, NULL);

    gettimeofday(&tv,NULL);
    L2 = tv.tv_sec*1000*1000 + tv.tv_usec;

    //排序结果
    result_list = g_slist_sort(result_list, compare_word);

    gettimeofday(&tv,NULL);
    L3 = tv.tv_sec*1000*1000 + tv.tv_usec;

    // 写入文件 TODO 比较一次循环和2次循环的速度

    // 释放树和链表
    freehzlist();
    freewordtree();
    freeresultlist();
    freeorphanlist();

    gettimeofday(&tv,NULL);
    L4 = tv.tv_sec*1000*1000 + tv.tv_usec;
    printf("clean:%lldus\nsort:%lldus\nfree:%lldus\n",L2-L1, L3-L2, L4-L3);
    munmap(mbuf, len);
    return 0;
}

//查询hanzi是否在全局字列表里
gboolean is_hanzi_in_list(WORDORIGIN hanzi)
{
    GSList *node;
    for(node=hanzi_list;node;node=node->next)
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

void add_word_to_list(WORDORIGIN word)
{
    //printaword(word);
    WORDORIGIN *k = g_new(WORDORIGIN, 1);
    int *v = g_new(int, 1);
    memcpy(k, &word, sizeof(WORDORIGIN));
    *v = 1;
    g_tree_insert(word_tree,k,v);
}
//查找出所有以hanzi开头的词
//start需是hanzi开头之处
void find_all_word(char *start, size_t len)
{
    long long L1,L2,L3,L4,L5;
    long long M1,M2,M3,M4,M5;
    struct timeval tv;

    char *p;
    int i;
    int size;
    int *pfreq;

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

            // 遇标点结束
            word = trim_punc(word);
            if(g_utf8_strlen(word.str, word.size) < 2) continue;

//    gettimeofday(&tv,NULL);
//    M2 = tv.tv_sec*1000*1000 + tv.tv_usec;
            if((pfreq = g_tree_lookup(word_tree, &word)) != NULL)
            {
//    gettimeofday(&tv,NULL);
//    M3 = tv.tv_sec*1000*1000 + tv.tv_usec;
                *pfreq += 1;
            }
            else
            {
//    gettimeofday(&tv,NULL);
//    M3 = tv.tv_sec*1000*1000 + tv.tv_usec;
                add_word_to_list(word);
            }
//    gettimeofday(&tv,NULL);
//    M4 = tv.tv_sec*1000*1000 + tv.tv_usec;
        }
//    gettimeofday(&tv,NULL);
//    L4 = tv.tv_sec*1000*1000 + tv.tv_usec;

        p = forward_a_char(p);
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
    for(node=hanzi_list;node;node=node->next)
    {
        free(node->data);
    }
}
void freewordtree()
{
    g_tree_destroy(word_tree);
}
void freeresultlist()
{
    printf("resultlist:%d\n",g_slist_length(result_list));
    int fd = open("/tmp/resultlist",O_WRONLY|O_CREAT|O_TRUNC,0600);
    char buf[512];
    char sword[256];
    AWORD *word;
    GSList *node;
    for(node=result_list;node;node=node->next)
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
    for(node=orphan_list;node;node=node->next)
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

static gint
word_compare(gconstpointer p1, gconstpointer p2, gpointer data)
{
    const WORDORIGIN *a = p1;
    const WORDORIGIN *b = p2;

    if(a->size != b->size)
        return a->size - b->size;
    else
        return strncmp(a->str,b->str,a->size);
}
static gint
tidy_traverse(gpointer key, gpointer value, gpointer data)
{
    WORDORIGIN *word = key;
    int *pfreq = value;
    gpointer *pd = data;
    AWORD *a = pd[0];
    gboolean *b = pd[1];

    char str[128],str_son[128];

    // 如存在包含a的且频率(一样)的词则返回TRUE
    if(*pfreq == a->freq && word->size > a->size)
    {
        strncpy(str, word->str, word->size);
        strncpy(str_son, a->str, a->size);
        if(strstr(str, str_son) != NULL)
        {
            *b = TRUE;
            return TRUE;
        }
    }
    return FALSE;
}
gboolean is_son(AWORD *a)
{
    gboolean b = FALSE;
    gpointer p[2] = {a, &b};
    g_tree_foreach(word_tree, tidy_traverse, &p[0]);
    return b;
}
static gint
clean_traverse(gpointer key, gpointer value, gpointer data)
{
    WORDORIGIN *word = key;
    int *pfreq = value;

    if(*pfreq > 1)
    {
        AWORD *a = g_new(AWORD,1);
        a->str = word->str;
        a->size = word->size;
        a->freq = *pfreq;

        if(g_utf8_strlen(word->str, word->size) == MAX_WORD_LEN)
        {
            orphan_list = g_slist_append(orphan_list, a);
            //TODO research orphan
        }
        else
        {
            // 存在包含关系且频率(一样)的词只保留最长的
            if(!is_son(a))
            {
                //printaword(*a);
                result_list = g_slist_append(result_list, a);
            }
        }
    }
    return FALSE;
}
