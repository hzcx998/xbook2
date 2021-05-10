#include <xbook/memcache.h>
#include <string.h>
#include <xbook/debug.h>

void string_init(string_t *string)
{
    string->length = 0;
    string->max_length = STRING_MAX_LEN;
    string->text = NULL;
}

/**
 * string_new - 创建化一个字符串
 * @string: 字符串对象
 * @text: 文本
 * @maxlen: 最大长度
 * 
 * @return: 成功返回0，失败返回-1
 */
int string_new(string_t *string, char *text, unsigned int maxlen)
{
    if (string == NULL || text == NULL || maxlen < 1)
        return -1;
    string->text = mem_alloc(maxlen);
    
    if (string->text == NULL) {
        return -1;
    }
        
    if (maxlen >= STRING_MAX_LEN) {
        maxlen = STRING_MAX_LEN - 1;
    }
    string->length = strlen(text);
    if (string->length > maxlen) {
        string->length = maxlen;
    }
    
    string->max_length = maxlen;
    memset(string->text, 0, maxlen);

    memcpy(string->text, text, string->length);
    string->text[string->length] = '\0';
    return 0;
}

/**
 * string_del - 删除字符串
 * 
 * 释放字符串占用的文本控件
 */
void string_del(string_t *string)
{
    if (string->text) {
        mem_free(string->text);
        string->text = NULL;
    }
    string->length = 0;
    string->max_length = STRING_MAX_LEN;
}

void string_copy(string_t *string, char *text)
{
    string->length = strlen(text);
    
    if (string->length > string->max_length) {
        string->length = string->max_length;
    }
    memcpy(string->text, text, string->length);
    string->text[string->length] = '\0'; 
}

void string_empty(string_t *string)
{
    memset(string->text, 0, string->max_length);
}

