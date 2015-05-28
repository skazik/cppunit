#ifndef ISOCK_CPPUNIT_H
#define ISOCK_CPPUNIT_H

#define CPPUNIT_TEST_COUNT_MAX       0xFF
#define CPPUNIT_TEST_RESULTS_MAX     100
#define CPPUNIT_TEST_PARAMS_LENGTH   0xFF
#define CPPUNIT_TEST_END_OF_LIST     250

typedef void (*executing_func)();
typedef struct _testListDesc {
    int test_index; // range [0..CPPUNIT_TEST_END_OF_LIST[
} TestListDesc_t;

typedef enum _TestName {
    eScriptValidation = 0,
    eFinalReturnValue,     // 1
    eExecute,              // 2
    eParseResult,          // [3 - 102] reserved for 100 results max
    eParseResultLast  = 102,
} eTestName_t;

extern void cppunit_selftest();
extern void cppunit_setprefix(const char *prefix);
extern void cppunit_compare_values(const char *file, int line, int expected, int retval, eTestName_t test_id, long long initime = 0, char *msg = NULL);
extern void cppunit_compare_listvals(const char *file, int line, int count, unsigned int vals[], eTestName_t test_id, long long initime = 0, char *msg = NULL);

#define CPPUNIT_SET_SCRIPT_PREFIX(__fn) cppunit_setprefix((const char*) __fn)
#define CPPUNIT_COMPARE_VALS(__exp, __rv, __testid) cppunit_compare_values(__FILE__, __LINE__, __exp, __rv, __testid)
#define CPPUNIT_COMPARE_VALS_TIME(__exp, __rv, __testid) cppunit_compare_values(__FILE__, __LINE__, __exp, __rv, __testid, UTILS_current_timestamp())
#define CPPUNIT_COMPARE_VALS_MESSAGE(__exp, __rv, __testid, __msg) cppunit_compare_values(__FILE__, __LINE__, __exp, __rv, __testid, UTILS_current_timestamp(), __msg)
#define CPPUNIT_COMPARE_VALS_LIST(__cnt, __vals, __testid) cppunit_compare_listvals(__FILE__, __LINE__, __cnt, __vals, __testid)

#endif // ISOCK_CPPUNIT_H
