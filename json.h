
#ifndef JSON_H
#define JSON_H

#include <stdlib.h>
#include <string.h>

#ifndef __cplusplus
typedef unsigned char                           bool;
#define true                                    (1)
#define false                                   (0)
#define TRUE                                    true
#define FALSE                                   false
#endif

struct _jsonobject;
struct _jsonpair;
union _jsonvalue;

typedef enum
{
    JSON_STRING = 0,
    JSON_OBJECT
} JSONValueType;

typedef struct _jsonobject
{
    struct _jsonpair *pairs;
    int count;
} JSONObject;

typedef struct _jsonpair
{
    char * key;
    union _jsonvalue *value;
    JSONValueType type;
} JSONPair;

typedef union _jsonvalue
{
    char * stringValue;
    struct _jsonobject *jsonObject;
} JSONValue;

extern JSONObject *parseJSON(const char *);
extern void freeJSONFromMemory(JSONObject *);

#endif

