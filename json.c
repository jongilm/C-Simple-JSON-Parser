// simple_json.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

/******************************************************************************************************************************************
 * Simple JSON Parser in C
 * Extremely simple JSON Parser library written in C
 *
 * Github:
 *    https://github.com/forkachild/C-Simple-JSON-Parser
 *
 * Features
 *     * Structured data (JSONObject, JSONPair, JSONValue)
 *     * Count of Key-Value pairs of current JSON
 *     * Recursive JSON parsing
 *     * JSONValue is a union whose type is stored as JSONValueType enum in its JSONPair
 *     * __BONUS__ string, bool and character data types introduced
 * Setup:
 *     Extremely simple setup.
 *     Just __copy__ `json.h` and `json.c` in your source folder and `#include "json.h"` in your source file
 *
 * Sample Code:
 *     See main() at the end of this file.
 *
 * FAQ:
 *     Q. What if JSON is poorly formatted with uneven whitespace
 *     A. Well, that is not a problem for this library
 *     Q. What if there is error in JSON
 *     A. That is when the function returns NULL
 *     Q. What is `_parseJSON` and how is it different from `parseJSON`
 *     A. '_parseJSON' is the internal `static` implementation not to be used outside the library
 *
 * If this helped you in any way you can buy me a beer at [PayPal](https://www.paypal.me/suhelchakraborty "Buy me a beer")
 ******************************************************************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "json.h"

#define my_new(x)                                  (x *)malloc(sizeof(x))
#define my_newWithSize(x, y)                       (x *)malloc((y) * sizeof(x))
#define my_renewWithSize(x, y, z)                  (y *)realloc((x), (z) * sizeof(y))
#define isWhitespace(x)                         ((x) == '\r' || (x) == '\n' || (x) == '\t' || (x) == ' ')
#define removeWhitespace(x)                     while(isWhitespace(*(x))) (x)++
#define removeWhitespaceCalcOffset(x, y)        while(isWhitespace(*(x))) { (x)++; (y)++; }

// Forward Declarations
static JSONObject * _parseJSON(const char *, int *);


JSONObject *parseJSON(const char * jsonString)
{
    int offset = 0;

    JSONObject *tempObj = _parseJSON(jsonString, &offset);

    return tempObj;
}

void freeJSONFromMemory(JSONObject *obj)
{
    int i;

    if (obj == NULL)
        return;

    if (obj->pairs == NULL)
    {
        free(obj);
        return;
    }

    for (i = 0; i < obj->count; i++)
    {
        if (obj->pairs[i].key != NULL)
            free(obj->pairs[i].key);
        if (obj->pairs[i].value != NULL)
        {
            switch (obj->pairs[i].type)
            {
                case JSON_STRING:
                    free(obj->pairs[i].value->stringValue);
                    break;
                case JSON_OBJECT:
                    freeJSONFromMemory(obj->pairs[i].value->jsonObject);
            }
            free(obj->pairs[i].value);
        }
    }
}

static int strNextOccurence(const char * str, char ch)
{
    int pos = 0;

    if (str == NULL)
        return -1;

    while (*str != ch && *str != '\0')
    {
        str++;
        pos++;
    }
    return (*str == '\0') ? -1 : pos;
}

static JSONObject * _parseJSON(const char * str, int *pOffset)
{
    int _offset = 0;

    // Create new object
    JSONObject *obj = my_new(JSONObject);
    // Set number of keyvaluepairs in this object
    obj->count = 1;
    // Create the first keyvalue pair
    obj->pairs = my_newWithSize(JSONPair, 1);

    while (*str != '\0')
    {
        removeWhitespaceCalcOffset(str, _offset);
        if (*str == '{')
        {
            // Start of new object
            // Skip over the opening curlybrace
            str++;
            _offset++;
        }
        else if (*str == '"')
        {
            // Start of String

            // Skip over opening doublequote char
            str++;
            // Find closing doublequote char
            int i = strNextOccurence(str, '"');
            if (i <= 0)
            {
                // Closing doublequote char not found for key
                freeJSONFromMemory(obj);
                return NULL;
            }

            // Store string as the key part of the keyvalue pair
            JSONPair tempPtr = obj->pairs[obj->count - 1];
            tempPtr.key = my_newWithSize(char, i + 1);
            memcpy(tempPtr.key, str, i * sizeof(char));
            tempPtr.key[i] = '\0';

            // Skip over the found string, and closing doublequote char
            str += i + 1;
            _offset += i + 2; // extra 1 for the opening doublequote char

            // Look for colon char, ignoring everything between the '"' and the ':'.
            // i.e. separator between key and value
            i = strNextOccurence(str, ':');
            if (i == -1)
            {
                // Colon char not found after key string
                return NULL;
            }
            // Skip over the ignored chars, and the colon
            str += i + 1;
            _offset += i + 1;

            removeWhitespaceCalcOffset(str, _offset);

            // Is the value another object, or a string?
            if (*str == '{')
            {
                int _offsetBeforeParsingChildObject = _offset;
                int _sizeOfChildObject;

                // The value is an object
                tempPtr.value = my_new(JSONValue);
                tempPtr.type = JSON_OBJECT;
                // Parse the object recursively
                tempPtr.value->jsonObject = _parseJSON(str, &_offset);
                if (tempPtr.value->jsonObject == NULL)
                {
                    // Value object not valid, for some or other reason
                    freeJSONFromMemory(obj);
                    return NULL;
                }
                // JG: Is this the problem. What is _offset ???
                // Advance the string pointer by the size of the processed child object
                _sizeOfChildObject = _offset - _offsetBeforeParsingChildObject;
                str += _sizeOfChildObject;
                //str += _offset;
            }
            else if (*str == '"')
            {
                // The value is a string
                // Skip over the doublequote char
                str++;
                // Look for the closing doublequote char
                i = strNextOccurence(str, '"');
                if (i == -1)
                {
                    // Cannot find closing double quote char for value string
                    freeJSONFromMemory(obj);
                    return NULL;
                }
                // Store the value string in the value portion of t he keyvalue pair
                tempPtr.value = my_new(JSONValue);
                tempPtr.type = JSON_STRING;
                tempPtr.value->stringValue = my_newWithSize(char, i + 1);
                memcpy(tempPtr.value->stringValue, str, i * sizeof(char));
                tempPtr.value->stringValue[i] = '\0';

                // Skip over the string and the closing doublequote
                str += i + 1;
                _offset += i + 2;  // extra 1 for the opening doublequote char
            }
            // Insert object into array of objects
            obj->pairs[obj->count - 1] = tempPtr;
        }
        else if (*str == ',')
        {
            // Start of the next keyvalue pair

            // Increment the number of keyvalue pairs
            obj->count++;
            // Add another keyvalue pair to the object
            obj->pairs = my_renewWithSize(obj->pairs, JSONPair, obj->count);

            // Skip over the comma
            str++;
            _offset++;
        }
        else if (*str == '}')
        {
            // End of object

            // Skip over the closing brace
            str++; // (pedantic)
            _offset++;

            // Update parent offset
            (*pOffset) += _offset;

            return obj;
        }
        // JG: Added else block
        else
        {
            // JG: Internal error (we should have caught all
            //     error possibilities, but justin case)
            freeJSONFromMemory(obj);
            return NULL;
        }
    }
    // JG: When does this happen??
    // JG: Premature end of input string???
    return obj;
}

#define RUN_SAMPLE_CODE
#ifdef RUN_SAMPLE_CODE
int main(int argc, const char * argv[])
{
    (void)argc;
    (void)argv;

    // Simple
    const char * someJsonString = "{\"hello\":\"world\",\"key\":\"value\"}";
    JSONObject *json1 = parseJSON(someJsonString);
    if (!json1)
    {
        printf("ERROR: Failed to parse JSON string\n");
        return 0;
    }
    printf("Count: %i\n", json1->count);                                  // Count: 2
    printf("Key: %s, Value: %s\n", json1->pairs[0].key,
                                   json1->pairs[0].value->stringValue);   // Key: hello, Value: world

    // Recursive
    const char * complexJsonString = "{\"name\":{\"first\":\"John\",\"last\":\"Doe\"},\"age\":\"21\"}";
    JSONObject *json2 = parseJSON(complexJsonString);
    if (!json2)
    {
        printf("ERROR: Failed to parse JSON string\n");
        return 0;
    }
    for (int ii=0; ii<json2->count; ii++)
    {
        printf("Key-Value pair %d is a %s\n", ii, (json2->pairs[ii].type == JSON_STRING)?"string":"JSON object");
    }
    //JSONObject *nameJson = json2->pairs[0].value->jsonObject;
    //printf("First name: %s\n", nameJson->pairs[0].value->stringValue);
    //printf("Last name: %s\n", nameJson->pairs[1].value->stringValue);

    // Some useful properties
    for (int ii=0; ii<json2->count; ii++)
    {
        printf("Key-Value pair %d is a %s\n", ii, (json2->pairs[ii].type == JSON_STRING)?"string":"JSON object");
    }
    return 0;
}

/*
{ "hello":"world", "key":"value" }
0----+----1----+----2----+----3-

{"name":{"first":"John", "last":"Doe"}, "age":"21"}
0----+----1----+----2----+----3----+----4----+----
        0----+----1----+----2----+----
*/
#endif 
